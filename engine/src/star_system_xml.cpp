#include <expat.h>
#include "xml_support.h"
#include "star_system_generic.h"
#include "cmd/planet_generic.h"
#include "cmd/unit_factory.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "configxml.h"
#include "vegastrike.h"
#include <assert.h>                              /// needed for assert() calls.
#include "gfx/mesh.h"
#include "cmd/building_generic.h"
#include "cmd/ai/aggressive.h"
#include "cmd/ai/fire.h"
#include "cmd/nebula_generic.h"
#include "cmd/asteroid_generic.h"
#include "cmd/enhancement_generic.h"
#include "cmd/script/flightgroup.h"
#include "universe_util.h"
#include "cmd/atmosphere.h"
#include "star_xml.h"
#include "planetary_orbit.h"

#include "options.h"

#include "system_factory.h"

#include <stdlib.h>

#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;
using std::cout;
using std::endl;

using namespace XMLSupport;
using namespace VSFileSystem;
extern const vector< string >& ParseDestinations( const string &value );
extern void bootstrap_draw( const string &message, Animation *SplashScreen = NULL );
extern void disableTerrainDraw( ContinuousTerrain *ct );

AtmosphericFogMesh::AtmosphericFogMesh()
{
    er              = eg = eb = ea = .5;
    dr              = dg = db = da = .5;
    meshname        = "sphereatm.bfxm";
    scale           = 1.05;
    focus           = .5;
    concavity       = 0;
    tail_mode_start = -1;
    tail_mode_end   = -1;
    min_alpha       = 0;
    max_alpha       = 255;
}




using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;

extern Flightgroup * getStaticBaseFlightgroup( int faction );
extern Flightgroup * getStaticNebulaFlightgroup( int faction );
extern Flightgroup * getStaticAsteroidFlightgroup( int faction );

template < typename T >
static bool EvalCondition( const char *op, const T &left, const T &right )
{
    switch (op[0])
    {
    case '>':
        switch (op[1])
        {
        case 0:
            return left > right;

        case '=':
            return left >= right;

        default:
            return false;
        }
    case '=':
        switch (op[1])
        {
        case 0:
        case '=':
            return left == right;

        default:
            return false;
        }
    case '<':
        switch (op[1])
        {
        case 0:
            return left < right;

        case '=':
            return left <= right;

        default:
            return false;
        }
    case '!':
        switch (op[1])
        {
        case '=':
            return left != right;

        default:
            return false;
        }
    default:
        return false;
    }
}

static bool ConfigCondition( const string &cond )
{
    if ( cond.empty() )
        return true;
    char varname[64];
    char op[3];
    char varval[64];
    bool ok = 3 == sscanf( cond.c_str(), "%63[-a-zA-Z_0-9] %2[<>=!] %63[-0-9.Ee]",
                           varname,
                           op,
                           varval );
    if (!ok)
        return false;
    //finalize character strings, for security
    varname[sizeof (varname)/sizeof (*varname)-1] = 0;
    op[sizeof (op)/sizeof (*op)-1] = 0;
    varval[sizeof (varval)/sizeof (*varval)-1]    = 0;
    //try to parse varval - if not parseable as float, assume it's a string
    char *endptr = 0;
    bool  rv;
    if (endptr == varval) {
        string sval = vs_config->getVariable( "graphics", varname, "0.0" );
        rv = EvalCondition< string > ( op, sval, varval );
    } else {
        float fval = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", varname, "0.0" ) );
        float fref = XMLSupport::parse_floatf( varval );
        rv = EvalCondition< float > ( op, fval, fref );
    }
    return rv;
}

bool ConfigAllows( string var, float val )
{
    bool invert = false;
    if (var.length() == 0)
        return true;
    if (var[0] == '-') {
        var    = var.substr( 1 );
        invert = true;
    }
    float x = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", var, "0.0" ) );
    if (var.length() == 0)
        return true;
    return invert ? -x >= val : x >= val;
}

Vector ComputeRotVel( float rotvel, const QVector &r, const QVector &s )
{
    if ( (r.i || r.j || r.k) && (s.i || s.j || s.k) ) {
        QVector retval = r.Cross( s );
        retval.Normalize();
        retval = retval*rotvel;
        return retval.Cast();
    } else {
        return Vector( 0, rotvel, 0 );
    }
}

void GetLights( const vector< GFXLight > &origlights, vector< GFXLightLocal > &curlights, const char *str, float lightSize )
{
    int   tint;
    char  isloc;
    char *tmp = strdup( str );
    GFXLightLocal lloc;
    char *st  = tmp;
    int   numel;
    while ( ( numel = sscanf( st, "%d%c", &tint, &isloc ) ) > 0 ) {
        assert( tint < (int) origlights.size() );
        lloc.ligh    = origlights[tint];
        lloc.islocal = (numel > 1 && isloc == 'l');

        lloc.ligh.setSize(lightSize);

        curlights.push_back( lloc );
        while ( isspace( *st ) )
            ++st;
        while ( isalnum( *st ) )
            ++st;
    }
    free( tmp );
}

extern Unit * getTopLevelOwner();
extern BLENDFUNC parse_alpha( const char* );
void SetSubunitRotation( Unit *un, float difficulty )
{
    Unit *unit;
    for (un_iter iter = un->getSubUnits(); (unit = *iter); ++iter) {
        float x = 2*difficulty*( (float) rand() )/RAND_MAX-difficulty;
        float y = 2*difficulty*( (float) rand() )/RAND_MAX-difficulty;
        float z = 2*difficulty*( (float) rand() )/RAND_MAX-difficulty;
        unit->SetAngularVelocity( Vector( x, y, z ) );
        SetSubunitRotation( unit, difficulty );
    }
}

void parse_dual_alpha( const char *alpha, BLENDFUNC &blendSrc, BLENDFUNC &blendDst )
{
    blendSrc = ONE;
    blendDst = ZERO;
    if (alpha == NULL) {} else if (alpha[0] == '\0') {} else {
        char *s = strdup( alpha );
        char *d = strdup( alpha );
        blendSrc = SRCALPHA;
        blendDst = INVSRCALPHA;
        if ( 2 == sscanf( alpha, "%s %s", s, d ) ) {
            if (strcmp( s, "true" ) != 0) {
                blendSrc = parse_alpha( s );
                blendDst = parse_alpha( d );
            }
        }
        free( s );
        free( d );
    }
}

int GetNumNearStarsScale()
{
    return game_options.num_near_stars;
}

int GetNumStarsScale()
{
    return game_options.num_far_stars;
}

float GetStarSpreadScale()
{
    return game_options.star_spreading*game_options.game_speed;
}

float ScaleOrbitDist( bool autogenerated )
{
    if (autogenerated)
        return game_options.autogen_compactness*game_options.runtime_compactness;
    return game_options.runtime_compactness;
}

extern float ScaleJumpRadius( float radius );

void StarSystem::processSystem(const AttributeList &attributes) {
    using namespace StarXML;
    AttributeList::const_iterator iter;
    assert( xml->unitlevel == 0 );
    ++xml->unitlevel;
    for (iter = attributes.begin(); iter != attributes.end(); ++iter) {
        switch ( attribute_map.lookup( (*iter).name ) )
        {
        case SCALESYSTEM:
            xml->scale *= parse_float( (*iter).value );
            break;
        case REFLECTIVITY:
            xml->reflectivity   = parse_float( (*iter).value );
            break;
        case BACKGROUND:
            xml->backgroundname = (*iter).value;
            break;
        case BACKGROUND_COLOR:
            parse_floatfv( (*iter).value, 4,
                &xml->backgroundColor.r,
                &xml->backgroundColor.g,
                &xml->backgroundColor.b,
                &xml->backgroundColor.a);
            break;
        case BACKGROUND_DEGAMMA:
            xml->backgroundDegamma = XMLSupport::parse_bool((*iter).value);
            break;

        case NAME:
            this->name = new char[strlen( (*iter).value.c_str() )+1];
            strcpy( this->name, (*iter).value.c_str() );
            break;

        case NEARSTARS:
        case STARS:
        case STARSPREAD:
        case FADESTARS:
        case X:
        case Y:
        case Z:
            break;
        }
    }
}

void StarSystem::processRing(const XMLSupport::AttributeList &attributes)
{
    using namespace StarXML;
    ++xml->unitlevel;
    string myfile( "planets/ring.png" );
    string varname;
    float varvalue = 0;
    BLENDFUNC blendSrc = SRCALPHA;
    BLENDFUNC blendDst = INVSRCALPHA;
    Unit *p = (Unit*) xml->moons.back()->GetTopPlanet( xml->unitlevel-1 );
    if (p != NULL) {
        if (p->isUnit() == PLANETPTR) {
            int   wrapx     = 1;
            int   wrapy     = 1;
            int   numslices = 8;
            float iradius   = p->rSize()*1.25f;
            float oradius   = p->rSize()*1.75f;
            QVector R( 1, 0, 0 );
            QVector S( 0, 1, 0 );
            AttributeList::const_iterator iter;
            for (iter = attributes.begin(); iter != attributes.end(); ++iter) {
                switch ( attribute_map.lookup( (*iter).name ) )
                {
                case XFILE:
                    myfile = (*iter).value;
                    break;
                case ALPHA:
                    parse_dual_alpha( (*iter).value.c_str(), blendSrc, blendDst );
                    break;
                case INNERRADIUS:
                    iradius   = parse_floatf( (*iter).value )*xml->scale;
                    break;
                case OUTERRADIUS:
                    oradius   = parse_floatf( (*iter).value )*xml->scale;
                    break;
                case NUMSLICES:
                    numslices = parse_int( (*iter).value );
                    break;
                case WRAPX:
                    wrapx     = parse_int( (*iter).value );
                    break;
                case WRAPY:
                    wrapy     = parse_int( (*iter).value );
                    break;
                case RI:
                    R.i      = parse_float( (*iter).value );
                    break;
                case RJ:
                    R.j      = parse_float( (*iter).value );
                    break;
                case RK:
                    R.k      = parse_float( (*iter).value );
                    break;
                case SI:
                    S.i      = parse_float( (*iter).value );
                    break;
                case SJ:
                    S.j      = parse_float( (*iter).value );
                    break;
                case SK:
                    S.k      = parse_float( (*iter).value );
                    break;
                case VARNAME:
                    varname  = (*iter).value;
                    break;
                case VARVALUE:
                    varvalue = parse_floatf( (*iter).value );
                    break;
                default:
                    break;
                }
            }
            if ( p != NULL && ConfigAllows( varname, varvalue ) )
                ( (Planet*) p )->AddRing( myfile, iradius, oradius, R, S, numslices, wrapx, wrapy, blendSrc, blendDst );
        }
    }
}

void StarSystem::processSpaceElevator(const XMLSupport::AttributeList &attributes,
                                      string truncatedfilename) {
    using namespace StarXML;
    AttributeList::const_iterator iter;
    ++xml->unitlevel;
    string myfile( "elevator" );
    string varname;
    float  varvalue = 0;
    BLENDFUNC blendSrc = SRCALPHA;
    BLENDFUNC blendDst = INVSRCALPHA;
    Unit *p = (Unit*) xml->moons.back()->GetTopPlanet( xml->unitlevel-1 );
    if (p != NULL && p->isUnit() == PLANETPTR) {
        string faction( UniverseUtil::GetGalaxyFaction( truncatedfilename ) );
        char   direction = 'b';

        QVector R( 1, 0, 0 );
        QVector S( 0, 1, 0 );
        for (iter = attributes.begin(); iter != attributes.end(); ++iter) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case XFILE:
                myfile   = (*iter).value;
                break;
            case VARNAME:
                varname  = (*iter).value;
                break;
            case VARVALUE:
                varvalue = parse_floatf( (*iter).value );
                break;

            case DIRECTION:
                if ( !(*iter).value.empty() )
                    direction = (*iter).value[0];
                break;
            case FACTION:
                faction = (*iter).value;
                if ( faction == UniverseUtil::GetGalaxyProperty( truncatedfilename, "faction" ) ) {
                    string ownerfaction = UniverseUtil::GetGalaxyFaction( truncatedfilename );
                    faction = ownerfaction;
                }
                break;
            default:
                break;
            }
        }
        if ( p != NULL && ConfigAllows( varname, varvalue ) )
            ( (Planet*) p )->AddSpaceElevator( myfile, faction, direction );

    }
}

void StarSystem::processFog(const XMLSupport::AttributeList &attributes) {
    using namespace StarXML;
    ++xml->unitlevel;

    if (!game_options.usePlanetFog)
        return;

    xml->fogopticalillusion = true;
    xml->fog.clear();
    AttributeList::const_iterator iter;
    for (iter = attributes.begin(); iter != attributes.end(); ++iter) {
        switch ( attribute_map.lookup( (*iter).name ) )
        {
        case OPTICALILLUSION:
            xml->fogopticalillusion = XMLSupport::parse_bool( iter->value );
            return;
        }
    }
}

void StarSystem::processFogElement(const XMLSupport::AttributeList &attributes) {
    using namespace StarXML;
    ++xml->unitlevel;

    if (!game_options.usePlanetFog)
        return;

    xml->fog.push_back( AtmosphericFogMesh() );
    xml->fog.back().scale = 1.1-.075+.075*xml->fog.size();

    AttributeList::const_iterator iter;
    for (iter = attributes.begin(); iter != attributes.end(); ++iter) {
        switch ( attribute_map.lookup( (*iter).name ) )
        {
        case EMRED:
            xml->fog.back().er = XMLSupport::parse_float( iter->value );
            break;
        case EMGREEN:
            xml->fog.back().eg = XMLSupport::parse_float( iter->value );
            break;
        case EMBLUE:
            xml->fog.back().eb = XMLSupport::parse_float( iter->value );
            break;
        case EMALPHA:
        case ALPHA:
            xml->fog.back().ea = XMLSupport::parse_float( iter->value );
            break;
        case DIRED:
            xml->fog.back().dr = XMLSupport::parse_float( iter->value );
            break;
        case DIGREEN:
            xml->fog.back().dg = XMLSupport::parse_float( iter->value );
            break;
        case DIBLUE:
            xml->fog.back().db = XMLSupport::parse_float( iter->value );
            break;
        case DIALPHA:
            xml->fog.back().da = XMLSupport::parse_float( iter->value );
            break;
        case XFILE:
            xml->fog.back().meshname  = iter->value;
            break;
        case MINALPHA:
            xml->fog.back().min_alpha = (int) (XMLSupport::parse_float( iter->value )*255);
            break;
        case MAXALPHA:
            xml->fog.back().max_alpha = (int) (XMLSupport::parse_float( iter->value )*255);
            break;
        case CONCAVITY:
            xml->fog.back().concavity = XMLSupport::parse_float( iter->value );
            break;
        case FOCUS:
            xml->fog.back().focus     = XMLSupport::parse_float( iter->value );
            break;
        case TAILMODESTART:
            xml->fog.back().tail_mode_start = (int) ( XMLSupport::parse_float( iter->value ) );
            break;
        case TAILMODEEND:
            xml->fog.back().tail_mode_end   = (int) ( XMLSupport::parse_float( iter->value ) );
            break;
        case SCALEATMOS:
            xml->fog.back().scale = XMLSupport::parse_float( iter->value );

        default:
            break;
        }
    }
}

void StarSystem::processCityLights(const XMLSupport::AttributeList &attributes)
{
    using namespace StarXML;
    string myfile( "planets/Dirt_light.png" );
    ++xml->unitlevel;

    BLENDFUNC blendSrc = SRCALPHA;
    BLENDFUNC blendDst = ONE;
    int   wrapx = 1;
    bool  inside_out = false;
    int   wrapy = 1;
    string varname;
    float varvalue = 0;

    Unit *p     = (Unit*) xml->moons.back()->GetTopPlanet( xml->unitlevel-1 );

    if (p != NULL) {
        if (p->isUnit() == PLANETPTR) {
            float radius = p->rSize();
            AttributeList::const_iterator iter;
            for (iter = attributes.begin(); iter != attributes.end(); ++iter) {
                switch ( attribute_map.lookup( (*iter).name ) )
                {
                case XFILE:
                    myfile = (*iter).value;
                    break;
                case ALPHA:
                    parse_dual_alpha( (*iter).value.c_str(), blendSrc, blendDst );
                    break;
                case RADIUS:
                    radius     = parse_float( (*iter).value )*xml->scale;
                    break;
                case WRAPX:
                    wrapx      = parse_int( (*iter).value );
                    break;
                case WRAPY:
                    wrapy      = parse_int( (*iter).value );
                    break;
                case INSIDEOUT:
                    inside_out = parse_bool( (*iter).value );
                    break;
                case VARNAME:
                    varname    = (*iter).value;
                    break;
                case VARVALUE:
                    varvalue   = parse_float( (*iter).value );
                    break;

                default:
                    break;
                }
            }
            if ( ConfigAllows( varname, varvalue ) )
                ( (Planet*) p )->AddCity( myfile, radius, wrapx, wrapy, blendSrc, blendDst, inside_out );
        }
    }
}

void StarSystem::processAtmosphere(const XMLSupport::AttributeList &attributes)
{
    using namespace StarXML;
    ++xml->unitlevel;

    if (!game_options.usePlanetAtmosphere)
        return;

    string myfile( "sol/earthcloudmaptrans.png" );
    BLENDFUNC blendSrc = SRCALPHA;
    BLENDFUNC blendDst = INVSRCALPHA;
    bool   inside_out = false;
    string varname;
    float varvalue = 0;

    Unit  *p = (Unit*) xml->moons.back()->GetTopPlanet( xml->unitlevel-1 );
    if (p != NULL) {
        if (p->isUnit() == PLANETPTR) {
            float radius = p->rSize()*1.075;
            AttributeList::const_iterator iter;
            for (iter = attributes.begin(); iter != attributes.end(); ++iter) {
                switch ( attribute_map.lookup( (*iter).name ) )
                {
                case XFILE:
                    myfile = (*iter).value;
                    break;
                case ALPHA:
                    parse_dual_alpha( (*iter).value.c_str(), blendSrc, blendDst );
                    break;
                case RADIUS:
                    radius     = parse_float( (*iter).value )*xml->scale;
                    break;
                case VARNAME:
                    varname    = (*iter).value;
                    break;
                case VARVALUE:
                    varvalue   = parse_float( (*iter).value );
                    break;
                case INSIDEOUT:
                    inside_out = parse_bool( (*iter).value );
                    break;

                default:
                    break;
                }
            }
            if ( ConfigAllows( varname, varvalue ) )
                ( (Planet*) p )->AddAtmosphere( myfile, radius, blendSrc, blendDst, inside_out );
        }
    }


}

void StarSystem::processTerrain(const XMLSupport::AttributeList &attributes)
{
    using namespace StarXML;
    ++xml->unitlevel;
    QVector S = QVector( 1, 0, 0 );
    QVector R = QVector( 0, 0, 1 );
    QVector pos = QVector( 0, 0, 0 );
    float radius = -1;
    float position = game_options.mass;
    float gravity = 0;
    float        scaleatmos = 10;
    int     numwraps = 1;
    float   scalex   = 1;
    string       myfile;

    AttributeList::const_iterator iter;
    for (iter = attributes.begin(); iter != attributes.end(); ++iter) {
        switch ( attribute_map.lookup( (*iter).name ) )
        {
        case NUMWRAPS:
            numwraps   = parse_int( (*iter).value );
            break;
        case SCALEX:
            scalex     = parse_float( (*iter).value );
            break;
        case SCALEATMOS:
            scaleatmos = parse_float( (*iter).value );
            break;
        case XFILE:
            myfile     = (*iter).value;
            break;
        case GRAVITY:
            gravity    = parse_float( (*iter).value );
            break;
        case MASS:
            position   = parse_float( (*iter).value );
            break;
        case RADIUS:
            radius     = parse_float( (*iter).value );
            break;
        case X:
            pos.i = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case Y:
            pos.j = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case Z:
            pos.k = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case RI:
            R.i   = parse_float( (*iter).value );
            break;
        case RJ:
            R.j   = parse_float( (*iter).value );
            break;
        case RK:
            R.k   = parse_float( (*iter).value );
            break;
        case QI:
            S.i   = parse_float( (*iter).value );
            break;
        case QJ:
            S.j   = parse_float( (*iter).value );
            break;
        case QK:
            S.k   = parse_float( (*iter).value );
            break;
        }
    }

    static Vector TerrainScale( game_options.xscale, game_options.yscale, game_options.zscale );
    Matrix t;
    Identity( t );
    float  y = S.Magnitude();
    Normalize( S );
    float  z = R.Magnitude();
    Normalize( R );
    TerrainScale.i *= z;
    TerrainScale.k *= z;
    TerrainScale.j *= y;
    t.r[3] = S.i*TerrainScale.j;
    t.r[4] = S.j*TerrainScale.j;
    t.r[5] = S.k*TerrainScale.j;
    t.r[6] = R.i*TerrainScale.k;
    t.r[7] = R.j*TerrainScale.k;
    t.r[8] = R.k*TerrainScale.k;
    S = S.Cross( R );
    t.r[0] = S.i*TerrainScale.i;
    t.r[1] = S.j*TerrainScale.i;
    t.r[2] = S.k*TerrainScale.i;
    t.p    = pos+xml->systemcentroid.Cast();
    Names   elem     = (Names) element_map.lookup( name );
    if ( myfile.length() ) {
        if (elem == TERRAIN) {
            terrains.push_back( UnitFactory::createTerrain( myfile.c_str(), TerrainScale, position, radius, t ) );
            xml->parentterrain = terrains.back();
        } else {
            contterrains.push_back( UnitFactory::createContinuousTerrain( myfile.c_str(), TerrainScale, position, t ) );
            xml->ct = contterrains.back();
            if (xml->unitlevel > 2) {
                assert( xml->moons.size() != 0 );
                Planet *p = xml->moons.back()->GetTopPlanet( xml->unitlevel-1 );
                if (p) {
                    disableTerrainDraw( xml->ct );
                    p->setTerrain( xml->ct, scalex, numwraps, scaleatmos );
                    PlanetaryTransform **tmpp = (PlanetaryTransform**) &xml->parentterrain;
                    p->getTerrain( *tmpp );
                }
            }
        }
    }
}

void StarSystem::processLightProp(const XMLSupport::AttributeList &attributes,
                                  LIGHT_TARGET target)
{
    using namespace StarXML;
    assert( xml->unitlevel == 2 );
    assert( xml->lights.size() );
    ++xml->unitlevel;

    GFXColor color( 0, 0, 0, 1 );

    AttributeList::const_iterator iter;
    for (iter = attributes.begin(); iter != attributes.end(); ++iter) {
        switch ( attribute_map.lookup( (*iter).name ) )
        {
        case EMRED:
            color.r = parse_float( (*iter).value );
            break;
        case EMGREEN:
            color.g = parse_float( (*iter).value );
            break;
        case EMBLUE:
            color.b = parse_float( (*iter).value );
            break;
        case EMALPHA:
            color.a = parse_float( (*iter).value );
            break;
        }
    }

    xml->lights.back().SetProperties( target, color );
}

void StarSystem::processPlanet(const XMLSupport::AttributeList &attributes,
                               std::string truncatedfilename)
{
    using namespace StarXML;
    // BeginElement


    // Enhancement
    assert( xml->unitlevel > 0 );
    ++xml->unitlevel;

    QVector S( 0, 1, 0 );
    QVector R( 0, 0, 1 );
    string filename;
    string citylights;
    string technique;
    BLENDFUNC blendSrc = ONE;
    BLENDFUNC blendDst = ZERO;
    map< string, string > paramOverrides;
    vector< string > dest;
    string  fullname = "unknw";
    bool isdest = false;
    string  unitname;
    bool insideout = false;
    vector< GFXLightLocal >curlights;

    // The following code snippet makes the planet Atlantis not glow
    // when starting the game. This is actually light reflection code.
    // If you move around, the other side is dark.
    static GFXColor planet_mat_ambient   = vs_config->getColor( "default", "planet_mat_ambient",
                                           GFXColor( 1.0,1.0,1.0,1.0 ) );
    static GFXColor planet_mat_diffuse   = vs_config->getColor( "default", "planet_mat_diffuse",
                                           GFXColor( 1.0,1.0,1.0,1.0 ) );
    static GFXColor planet_mat_specular  = vs_config->getColor( "default", "planet_mat_specular",
                                           GFXColor( 0.0,0.0,0.0,1.0 ) );
    static GFXColor planet_mat_emissive = vs_config->getColor( "default", "planet_mat_emmissive",
                                           GFXColor( 0.0,0.0,0.0,1.0 ) );
    GFXMaterial ourmat;
    GFXGetMaterial( 0, ourmat );
    setMaterialAmbient(ourmat, planet_mat_ambient);
    setMaterialDiffuse(ourmat, planet_mat_diffuse);
    setMaterialSpecular(ourmat, planet_mat_specular);
    setMaterialEmissive(ourmat, planet_mat_emissive);
    // End planet not glow code

    float   gravity  = 0;
    float   velocity = 0;
    float radius;
    float   position = 0;
    float   rotvel   = 0;
    int faction = 0;

    AttributeList::const_iterator iter;
    for (iter = attributes.begin(); iter != attributes.end(); ++iter) {
        switch ( attribute_map.lookup( (*iter).name ) )
        {
        case NAME:
            fullname = (*iter).value;
            bootstrap_draw( "Loading "+fullname );
            break;
        case XFILE:
            filename = (*iter).value;
            break;
        case DESTINATION:
            dest   = ParseDestinations( (*iter).value );
            isdest = true;
            break;
        case TECHNIQUE:
            technique = (*iter).value;
            break;
        case UNIT:
            unitname = (*iter).value;
            break;
        case ALPHA:
            parse_dual_alpha( (*iter).value.c_str(), blendSrc, blendDst );
            break;
        case CITYLIGHTS:
            citylights = (*iter).value;
            break;

        case INSIDEOUT:
            insideout = XMLSupport::parse_bool( (*iter).value );
            break;
        case LIGHT:
            GetLights( xml->lights, curlights, (*iter).value.c_str(), radius );
            break;
        case FACTION:
            {
                int originalowner =
                    FactionUtil::GetFactionIndex( UniverseUtil::GetGalaxyProperty( truncatedfilename, "faction" ) );
                faction = FactionUtil::GetFactionIndex( (*iter).value );
                if (faction == originalowner) {
                    int ownerfaction = FactionUtil::GetFactionIndex( UniverseUtil::GetGalaxyFaction( truncatedfilename ) );
                    faction = ownerfaction;
                }
                break;
            }
        case EMRED:
            ourmat.er    = parse_float( (*iter).value );
            break;
        case EMGREEN:
            ourmat.eg    = parse_float( (*iter).value );
            break;
        case EMBLUE:
            ourmat.eb    = parse_float( (*iter).value );
            break;
        case EMALPHA:
            ourmat.ea    = parse_float( (*iter).value );
            break;
        case DIRED:
            ourmat.dr    = parse_float( (*iter).value );
            break;
        case DIGREEN:
            ourmat.dg    = parse_float( (*iter).value );
            break;
        case DIBLUE:
            ourmat.db    = parse_float( (*iter).value );
            break;
        case DIALPHA:
            ourmat.da    = parse_float( (*iter).value );
            break;
        case SRED:
            ourmat.sr    = parse_float( (*iter).value );
            break;
        case SGREEN:
            ourmat.sg    = parse_float( (*iter).value );
            break;
        case SBLUE:
            ourmat.sb    = parse_float( (*iter).value );
            break;
        case SALPHA:
            ourmat.sa    = parse_float( (*iter).value );
            break;
        case SPOWER:
            ourmat.power = parse_float( (*iter).value );
            break;
        case REFLECTNOLIGHT:
            ourmat.sr    = ourmat.sg = ourmat.sb = ourmat.dr = ourmat.dg = ourmat.db =
                                                                               ourmat.ar = ourmat.ag = ourmat.ab = 0;
            break;
        case RI:
            R.i = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case RJ:
            R.j = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case RK:
            R.k = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case SI:
            S.i = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case SJ:
            S.j = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case SK:
            S.k = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case X:
            xml->cursun.i = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;

        case Y:
            xml->cursun.j = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;

        case Z:
            xml->cursun.k = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;

        case RADIUS:
            radius   = parse_float( (*iter).value );
            break;
        case PPOSITION:
            position = parse_float( (*iter).value );
            break;
        case DAY:
            if (fabs( parse_float( (*iter).value ) ) > .00001)
                rotvel = 2.0f*M_PI/( game_options.YearScale*parse_floatf( (*iter).value ) );
            break;
        case YEAR:
            if (fabs( parse_float( (*iter).value ) ) > .00001)
                velocity = 2*M_PI/( game_options.YearScale*parse_float( (*iter).value ) );
            break;
        case GRAVITY:
            gravity = parse_float( (*iter).value );
            break;

        case OVERRIDE:
            {
                string::size_type eqpos = (*iter).value.find_first_of('=');
                if (eqpos != string::npos) {
                    string name = (*iter).value.substr(0,eqpos);
                    string value = (*iter).value.substr(eqpos+1);
                    paramOverrides[name] = value;
                }
            }
        }
    }


    cout << fullname << " ourmat (old): " << ourmat.sr << " : " << ourmat.sg << " : " << ourmat.sb << " : " <<
            ourmat.dr << " : " << ourmat.dg << " : " << ourmat.db << " : " << ourmat.ar <<
            " : " << ourmat.ag << " : " << ourmat.ab << endl;

    for(auto& local_light : curlights) {
        cout << "Light(orig) " << 7 << " : " << local_light.islocal << endl;
        auto& light = local_light.ligh;
        cout << "Ambient: " << light.ambient[0] << ":" << light.ambient[1] << ":"
                 << light.ambient[2] << ":" << light.ambient[3] << endl;
        cout << "Diffuse: " << light.diffuse[0] << ":" << light.diffuse[1] << ":"
                 << light.diffuse[2] << ":" << light.diffuse[3] << endl;
        cout << "Specular: " << light.specular[0] << ":" << light.specular[1] << ":"
                 << light.specular[2] << ":" << light.specular[3] << endl;
    }

    if (isdest == true)
        if (xml->fade)                       //xml->fade saves if it's autogenerated system
            radius = ScaleJumpRadius( radius );
    radius *= xml->scale;
    if (xml->unitlevel > 2) {
        assert( xml->moons.size() != 0 );
        fprintf(stderr, "Creating planet %s with texture %s and technique %s - unitlevel > 2\n", fullname.c_str(), filename.c_str(), technique.c_str());
        Unit *un = xml->moons[xml->moons.size()-1]->beginElement( R, S, velocity, ComputeRotVel( rotvel, R, S ),
                                                                  position, gravity, radius,
                                                                  filename, technique, unitname, blendSrc, blendDst, dest,
                                                                  xml->unitlevel-1, ourmat, curlights, false,
                                                                  faction
                                                                  != 0 ? faction : FactionUtil::GetFactionIndex(
                                                                        UniverseUtil::GetGalaxyFaction( truncatedfilename ) ),
                                                                  fullname, insideout );
        if (un) {
            un->SetOwner( getTopLevelOwner() );
            un->applyTechniqueOverrides(paramOverrides);
        }
    } else {
        Planet *planet;
        fprintf(stderr, "Creating planet %s with texture %s and technique %s - unitlevel <= 2\n", fullname.c_str(), filename.c_str(), technique.c_str());
        xml->moons.push_back( ( planet =
                                   UnitFactory::createPlanet( R, S, velocity,
                                                              ComputeRotVel( rotvel, R, S ),
                                                              position, gravity, radius,
                                                              filename, technique, unitname,
                                                              blendSrc, blendDst, dest, xml->cursun.Cast()
                                                              +xml->systemcentroid.Cast(),
                                                              NULL, ourmat, curlights, faction
                                                              != 0 ? faction : FactionUtil::GetFactionIndex(
                                                                    UniverseUtil::GetGalaxyFaction( truncatedfilename ) ),
                                                              fullname,
                                                              insideout ) ) );

        xml->moons[xml->moons.size()-1]->SetPosAndCumPos( R+S+xml->cursun.Cast()+xml->systemcentroid.Cast() );
        xml->moons.back()->SetOwner( getTopLevelOwner() );
        planet->applyTechniqueOverrides(paramOverrides);
    }
}

void StarSystem::processEnhancement(const XMLSupport::AttributeList &attributes,
                                    std::string truncatedfilename,
                                    int elem)
{
    using namespace StarXML;

    assert( xml->unitlevel > 0 );
    ++xml->unitlevel;

    QVector S( 0, 1, 0 );
    QVector R( 0, 0, 1 );

    char        * nebfile     = new char[1];
    nebfile[0]  = '\0';

    string filename = "";
    string fullname = "unkn-unit";
    string varname;
    string condition;

    vector< string > dest;

    int faction = 0;
    int neutralfaction = FactionUtil::GetNeutralFaction();

    float scalex = game_options.AsteroidDifficulty;
    float   velocity = 0;
    float   rotvel   = 0;
    float   position = 0;
    float varvalue = 0;


    AttributeList::const_iterator iter;
    for (iter = attributes.begin(); iter != attributes.end(); ++iter) {
        switch ( attribute_map.lookup( (*iter).name ) )
        {
        case NAME:
            fullname = (*iter).value;
            break;
        case XFILE:
            filename = (*iter).value;
            break;
        case NEBFILE:
            delete[] nebfile;
            nebfile = new char[strlen( (*iter).value.c_str() )+1];
            strcpy( nebfile, (*iter).value.c_str() );
            break;
        case DIFFICULTY:
            scalex  = -parse_float( (*iter).value );
            break;
        case DESTINATION:
            dest    = ParseDestinations( (*iter).value );
            break;
        case FACTION:
            faction = FactionUtil::GetFactionIndex( (*iter).value );
            {
                int originalowner =
                    FactionUtil::GetFactionIndex( UniverseUtil::GetGalaxyProperty( truncatedfilename, "faction" ) );
                if (faction == originalowner) {
                    int ownerfaction = FactionUtil::GetFactionIndex( UniverseUtil::GetGalaxyFaction( truncatedfilename ) );
                    faction = ownerfaction;
                }
            }
            break;
        case RI:
            R.i = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case RJ:
            R.j = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case RK:
            R.k = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case SI:
            S.i = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case SJ:
            S.j = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case SK:
            S.k = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case X:
            xml->cursun.i = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case Y:
            xml->cursun.j = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case Z:
            xml->cursun.k = parse_float( (*iter).value )*xml->scale*ScaleOrbitDist( xml->fade );
            break;
        case PPOSITION:
            position = parse_floatf( (*iter).value );
            break;
        case DAY:
            if (fabs( parse_floatf( (*iter).value ) ) > .00001)
                rotvel = 2.0f*M_PI/( game_options.YearScale*parse_floatf( (*iter).value ) );
            break;
        case YEAR:
            if (fabs( parse_floatf( (*iter).value ) ) > .00001)
                velocity = 2.0f*M_PI/( game_options.YearScale*parse_floatf( (*iter).value ) );
            break;
        case VARNAME:
            varname   = (*iter).value;
            break;
        case VARVALUE:
            varvalue  = parse_floatf( (*iter).value );
            break;
        case CONDITION:
            condition = (*iter).value;
            break;
        }
    }
    cout << "old: " <<elem << endl;
    cout << "R/X: " << R.i << " : " << R.j << " : " << R.k << endl;
    cout << "S/Y: " << S.i << " : " << S.j << " : " << S.k << endl;

    if ( ( !xml->conditionStack.size() || xml->conditionStack.back() )
        && ConfigAllows( varname, varvalue )
        && ConfigCondition( condition ) ) {

        if ( ( (elem == UNIT || elem == NEBULA || elem == ENHANCEMENT
                || elem == ASTEROID) || (xml->ct == NULL && xml->parentterrain == NULL) ) && (xml->unitlevel > 2) ) {
            assert( xml->moons.size() != 0 );
            Unit   *un = NULL; //FIXME !!! un appears to never be allocated memory !!! "= NULL" added by chuck_starchaser
            Planet *plan = xml->moons.back()->GetTopPlanet( xml->unitlevel-1 );
            if (elem == UNIT) {
                Flightgroup *fg = getStaticBaseFlightgroup( faction );
                plan->AddSatellite( un = UnitFactory::createUnit( filename.c_str(), false, faction, "", fg, fg->nr_ships-1 ) );
                un->setFullname( fullname ); //FIXME un de-referenced before allocation
            } else if (elem == NEBULA) {
                Flightgroup *fg = getStaticNebulaFlightgroup( faction );
                plan->AddSatellite( un = UnitFactory::createNebula( filename.c_str(), false, faction, fg, fg->nr_ships-1 ) );
            } else if (elem == ASTEROID) {
                Flightgroup *fg = getStaticAsteroidFlightgroup( faction );
                plan->AddSatellite( un =
                                       UnitFactory::createAsteroid( filename.c_str(), faction, fg, fg->nr_ships-1, scalex
                                                                    < 0 ? -scalex : scalex ) );
            } else if (elem == ENHANCEMENT) {
                plan->AddSatellite( un = UnitFactory::createEnhancement( filename.c_str(), faction, string( "" ) ) );
            }
            {
                for (unsigned int i = 0; i < dest.size(); ++i)
                    un->AddDestination( dest[i] ); //FIXME un de-referenced before allocation
                dest.clear();
            }
            un->SetAI( new PlanetaryOrbit( un, velocity, position, R, S, QVector( 0, 0, 0 ), plan ) );
            if (elem == UNIT && un->faction != neutralfaction) {
                un->SetTurretAI(); //FIXME un de-referenced before allocation
                un->EnqueueAI( new Orders::FireAt( 15 ) ); //FIXME un de-referenced before allocation
            }
            if (scalex < 0 && elem != ASTEROID)
                SetSubunitRotation( un, -scalex ); //FIXME un de-referenced before allocation
            //cheating so nothing collides at top lev
            un->SetOwner( getTopLevelOwner() ); //FIXME un de-referenced before allocation
            un->SetAngularVelocity( ComputeRotVel( rotvel, R, S ) ); //FIXME un de-referenced before allocation
        } else {
            if ( (elem == BUILDING || elem == VEHICLE) && xml->ct == NULL && xml->parentterrain != NULL ) {
                Unit *b = UnitFactory::createBuilding(
                        xml->parentterrain, elem == VEHICLE, filename.c_str(), false, faction, string("") );
                b->SetPosAndCumPos( xml->cursun.Cast()+xml->systemcentroid.Cast() );
                b->EnqueueAI( new Orders::AggressiveAI( "default.agg.xml" ) );
                AddUnit( b );
                {
                    for (unsigned int i = 0; i < dest.size(); ++i)
                        b->AddDestination( dest[i] );
                    dest.clear();
                }
            } else if ( (elem == BUILDING || elem == VEHICLE) && xml->ct != NULL ) {
                Unit *b = UnitFactory::createBuilding(
                        xml->ct, elem == VEHICLE, filename.c_str(), false, faction );
                b->SetPosAndCumPos( xml->cursun.Cast()+xml->systemcentroid.Cast() );
                b->EnqueueAI( new Orders::AggressiveAI( "default.agg.xml" ) );
                b->SetTurretAI();
                b->EnqueueAI( new Orders::FireAt( 15 ) );
                AddUnit( b );
                {
                    for (unsigned int i = 0; i < dest.size(); ++i)
                        b->AddDestination( dest[i] );
                    dest.clear();
                }
            } else {
                if (elem == UNIT) {
                    Flightgroup *fg = getStaticBaseFlightgroup( faction );
                    Unit *moon_unit = UnitFactory::createUnit( filename.c_str(), false, faction, "", fg, fg->nr_ships-1 );
                    moon_unit->setFullname( fullname );
                    xml->moons.push_back( (Planet*) moon_unit );
                } else if (elem == NEBULA) {
                    Flightgroup *fg = getStaticNebulaFlightgroup( faction );
                    xml->moons.push_back( (Planet*) UnitFactory::createNebula( filename.c_str(), false, faction, fg, fg->nr_ships-1 ) );
                } else if (elem == ASTEROID) {
                    Flightgroup *fg = getStaticAsteroidFlightgroup( faction );
                    Planet *ast;
                    xml->moons.push_back( ast =
                                             (Planet*) UnitFactory::createAsteroid( filename.c_str(), faction, fg, fg->nr_ships-1,
                                                                                    scalex ) );
                } else if (elem == ENHANCEMENT) {
                    Planet *enh = (Planet*) UnitFactory::createEnhancement( filename.c_str(), faction, string( "" ) );
                    xml->moons.push_back( enh );
                }
                {
                    Unit *un = xml->moons.back();
                    for (unsigned int i = 0; i < dest.size(); ++i)
                        un->AddDestination( dest[i] );
                    dest.clear();
                }
                xml->moons.back()->SetAI( new PlanetaryOrbit( xml->moons[xml->moons.size()-1], velocity, position, R, S,
                                                              xml->cursun.Cast()+xml->systemcentroid.Cast(), NULL ) );

                xml->moons.back()->SetPosAndCumPos( R+S+xml->cursun.Cast()+xml->systemcentroid.Cast() );
                xml->moons.back()->SetOwner( getTopLevelOwner() );
                if (elem == UNIT && xml->moons.back()->faction != neutralfaction) {
                    xml->moons.back()->SetTurretAI();
                    xml->moons.back()->EnqueueAI( new Orders::FireAt( 15 ) );
                } else if (scalex < 0 && elem != ASTEROID) {
                    SetSubunitRotation( xml->moons.back(), -scalex );
                }
            }
        }
    }
    delete[] nebfile;
}

void StarSystem::beginElement( const string &name, const AttributeList &attributes )
{
    using namespace StarXML;
    int          neutralfaction = FactionUtil::GetNeutralFaction();
    string       myfile;
    vector< GFXLightLocal >curlights;
    xml->cursun.i = 0;
    string       varname, condition;
    float        varvalue   = 0;
    GFXColor     tmpcol( 0, 0, 0, 1 );
    LIGHT_TARGET tmptarg    = POSITION;
    xml->cursun.j = 0;
    string       citylights;
    string       technique;
    float        scaleatmos = 10;
    char        *nebfile;
    bool         insideout  = false;
    int          faction    = 0;
    string       truncatedfilename = this->filename;
    {
        string::size_type tmp;
        if ( ( tmp = truncatedfilename.find( ".system" ) ) != string::npos )
            truncatedfilename = truncatedfilename.substr( 0, tmp );
    }

    BLENDFUNC blendSrc = ONE;
    BLENDFUNC blendDst = ZERO;
    bool isdest = false;

    xml->cursun.k = 0;
    static GFXColor planet_mat_ambient   = vs_config->getColor( "default", "planet_mat_ambient",
                                           GFXColor( 1.0,1.0,1.0,1.0 ) );
    static GFXColor planet_mat_diffuse   = vs_config->getColor( "default", "planet_mat_diffuse",
                                           GFXColor( 1.0,1.0,1.0,1.0 ) );
    static GFXColor planet_mat_specular  = vs_config->getColor( "default", "planet_mat_specular",
                                           GFXColor( 0.0,0.0,0.0,1.0 ) );
    static GFXColor planet_mat_emissive = vs_config->getColor( "default", "planet_mat_emmissive",
                                           GFXColor( 0.0,0.0,0.0,1.0 ) );
    GFXMaterial ourmat;
    GFXGetMaterial( 0, ourmat );
    setMaterialAmbient(ourmat, planet_mat_ambient);
    setMaterialDiffuse(ourmat, planet_mat_diffuse);
    setMaterialSpecular(ourmat, planet_mat_specular);
    setMaterialEmissive(ourmat, planet_mat_emissive);

    int     numwraps = 1;
    float   scalex   = 1;
    vector< string >dest;
    map< string, string > paramOverrides;
    string  filename;
    string  unitname;
    string  fullname = "unknw";
    float   gravity  = 0;
    float   velocity = 0;
    float   position = 0;
    float   rotvel   = 0;
    QVector S( 0, 0, 0 ), R( 0, 0, 0 );
    QVector pos( 0, 0, 0 );
    Names   elem     = (Names) element_map.lookup( name );
    float   radius   = 1.0f;
    AttributeList::const_iterator iter;
    switch (elem)
    {
    case UNKNOWN:
        ++xml->unitlevel;

        return;

    case SYSTEM:
        processSystem(attributes);

        break;
    case RING:
        processRing(attributes);
        break;

    case SPACEELEVATOR:
        processSpaceElevator(attributes, truncatedfilename);
        break;

    case FOG:
        processFog(attributes);
        break;

    case FOGELEMENT:
        processFogElement(attributes);
        break;

    case CITYLIGHTS:
        processCityLights(attributes);
        break;

    case ATMOSPHERE:
        processAtmosphere(attributes);
        break;

    case TERRAIN:
    case CONTTERRAIN:
        processTerrain(attributes);
        break;

    case LIGHT:
        assert( xml->unitlevel == 1 );
        ++xml->unitlevel;
        xml->lights.push_back( GFXLight( true, Vector( 0, 0, 0 ) ) );
        break;
    case ATTEN:
        processLightProp(attributes, ATTENUATE);
        break;

    case AMB:
        processLightProp(attributes, AMBIENT);
        break;

    case SPEC:
        processLightProp(attributes, SPECULAR);
        break;

    case DIFF:
        processLightProp(attributes, DIFFUSE);
        break;

    case JUMP:
    case PLANET:
        processPlanet(attributes, truncatedfilename);
        break;

    case CONDITION:
        for (iter = attributes.begin(); iter != attributes.end(); ++iter) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case EXPRESSION:
                condition = (*iter).value;
                break;
            }
        }
        xml->conditionStack.push_back(
            ( !xml->conditionStack.size() || xml->conditionStack.back() )
            && ConfigCondition( condition )
                                     );
        break;
    case UNIT:
    case BUILDING:
    case VEHICLE:
    case NEBULA:
    case ASTEROID:
    case ENHANCEMENT:
        processEnhancement(attributes, truncatedfilename, elem);

        break;
    default:
        break;
    }
}

void StarSystem::endElement( const string &name )
{
    using namespace StarXML;
    Names elem = (Names) element_map.lookup( name );
    switch (elem)
    {
    case FOG:
        {
            --xml->unitlevel;
            if (!game_options.usePlanetFog)
                break;
            Unit *p = (Unit*) xml->moons.back()->GetTopPlanet( xml->unitlevel );
            if (p != NULL)
                if (p->isUnit() == PLANETPTR)
                    ( (Planet*) p )->AddFog( xml->fog, xml->fogopticalillusion );
            break;
        }
    case UNKNOWN:
        --xml->unitlevel;
        break;
    case CONTTERRAIN:
        --xml->unitlevel;
        xml->ct = NULL;
        break;
    case TERRAIN:
        xml->parentterrain = NULL;
        --xml->unitlevel;
        break;
    case CONDITION:
        if ( xml->conditionStack.size() )
            xml->conditionStack.pop_back();
        break;
    default:
        --xml->unitlevel;
        break;
    }
    if (xml->unitlevel == 0) {}
}

void StarSystem::LoadXML( const char *filename, const Vector &centroid, const float timeofyear )
{
    using namespace StarXML;
    bool   autogenerated = false;
    this->filename = filename;
    string file = VSFileSystem::GetCorrectStarSysPath( filename, autogenerated );
    if ( file.empty() )
        file = filename;

    if (game_options.game_speed_affects_autogen_systems)
        autogenerated = false;

    xml = new Star_XML;
    xml->scale = 1;
    xml->fade  = autogenerated;
    xml->scale *= game_options.star_system_scale;
    xml->parentterrain = NULL;
    xml->ct     = NULL;
    xml->systemcentroid = centroid;
    xml->timeofyear     = timeofyear;
    xml->starsp = GetStarSpreadScale();
    xml->numnearstars   = GetNumNearStarsScale();
    xml->numstars = GetNumStarsScale();
    xml->backgroundname = string( "cube" );
    xml->backgroundColor = GFXColor(1.0,1.0,1.0,1.0);
    xml->backgroundDegamma = false;
    xml->reflectivity   = game_options.reflectivity;
    xml->unitlevel = 0;


    VSFile other_file;
    string full_path = other_file.GetSystemDirectoryPath(file);
    SystemFactory sys = SystemFactory(file, full_path, xml);

    for(auto& unit : xml->moons)
    {
        if (unit->isUnit() == PLANETPTR) {
            Unit *un = nullptr;
            // This code here is completely unclear to me and should be refactored
            // TODO: remove the whole PlanetIterator thing and all custom iterators
            for (Planet::PlanetIterator iter( (Planet*) unit ); (un = *iter); iter.advance() )
                AddUnit( un );
        } else {
            AddUnit( unit );
        }
    }

    createBackground( xml );
    delete xml;
}


/*void StarSystem::LoadXML( const char *filename, const Vector &centroid, const float timeofyear )
{
    using namespace StarXML;
    bool   autogenerated = false;
    this->filename = filename;
    string file = VSFileSystem::GetCorrectStarSysPath( filename, autogenerated );
    if ( file.empty() )
        file = filename;




    // Old code
    if (game_options.game_speed_affects_autogen_systems)
        autogenerated = false;
    VSFile  f;
    VSError err;
    //if (file.length()) {
    err = f.OpenReadOnly( file, SystemFile );
    if (err > Ok) {
        printf( "StarSystem: file not found %s\n", file.c_str() );
        return;
    }
    xml = new Star_XML;
    xml->scale = 1;
    xml->fade  = autogenerated;
    xml->scale *= game_options.star_system_scale;
    xml->parentterrain = NULL;
    xml->ct     = NULL;
    xml->systemcentroid = centroid;
    xml->timeofyear     = timeofyear;
    xml->starsp = GetStarSpreadScale();
    xml->numnearstars   = GetNumNearStarsScale();
    xml->numstars = GetNumStarsScale();
    xml->backgroundname = string( "cube" );
    xml->backgroundColor = GFXColor(1.0,1.0,1.0,1.0);
    xml->backgroundDegamma = false;
    xml->reflectivity   = game_options.reflectivity;
    xml->unitlevel = 0;

    XML_Parser parser = XML_ParserCreate( nullptr );
    XML_SetUserData( parser, this );
    XML_SetElementHandler( parser, &StarSystem::beginElement, &StarSystem::endElement );

    string fcontents = f.ReadFull();
    BOOST_LOG_TRIVIAL(debug) << "Contents of star system:";
    BOOST_LOG_TRIVIAL(debug) << fcontents;
    XML_Parse( parser, (fcontents).c_str(), f.Size(), 1 );

    f.Close();
    XML_ParserFree( parser );

    // New code
    Star_XML* xml2 = new Star_XML;
    xml2 = new Star_XML;
    xml2->scale = 1;
    xml2->fade  = autogenerated;
    xml2->scale *= game_options.star_system_scale;
    xml2->parentterrain = NULL;
    xml2->ct     = NULL;
    xml2->systemcentroid = centroid;
    xml2->timeofyear     = timeofyear;
    xml2->starsp = GetStarSpreadScale();
    xml2->numnearstars   = GetNumNearStarsScale();
    xml2->numstars = GetNumStarsScale();
    xml2->backgroundname = string( "cube" );
    xml2->backgroundColor = GFXColor(1.0,1.0,1.0,1.0);
    xml2->backgroundDegamma = false;
    xml2->reflectivity   = game_options.reflectivity;
    xml2->unitlevel = 0;

    VSFile other_file;
    string full_path = other_file.GetSystemDirectoryPath(file);
    SystemFactory sys = SystemFactory(file, full_path, xml2);

    // End of new code





    for(auto& unit : xml2->moons)
    {
        cout << "Adding unit " << unit->getFullname() << endl;
        if (unit->isUnit() == PLANETPTR) {
            Unit *un = nullptr;
            // This code here is completely unclear to me and should be refactored
            // TODO: remove the whole PlanetIterator thing and all custom iterators
            for (Planet::PlanetIterator iter( (Planet*) unit ); (un = *iter); iter.advance() )
                AddUnit( un );
        } else {
            AddUnit( unit );
        }
    }

    createBackground( xml2 );
    delete xml;
    delete xml2;
}*/
