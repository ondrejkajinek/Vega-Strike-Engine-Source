#include "vegastrike.h"
#include <vector>
#include "beam.h"
#include "unit_generic.h"
//#include "unit_collide.h"
using std::vector;
#include "audiolib.h"
#include "configxml.h"
#include "images.h"
//static DecalQueue beamdecals;
extern double interpolation_blend_factor;
extern bool AdjustMatrix(Matrix &mat,const Vector &velocity, Unit * target, float speed, bool lead, float cone);
float Beam::refireTime() {
	return refiretime;
}
void Beam::SetPosition (const QVector &k) {
  local_transformation.position = k;
}
void Beam::SetOrientation(const Vector &p, const Vector &q, const Vector &r)
{	
  local_transformation.orientation = Quaternion::from_vectors(p,q,r);
}
void ScaleByAlpha (GFXColorVertex &vert, float alpha) {
	if (alpha<1) {
		vert.r*=alpha;
		vert.g*=alpha;
		vert.b*=alpha;		
	}
}
void SetColorToVertex (GFXColorVertex &vert,const GFXColor &col=GFXColor(0,0,0,0)) {
	vert.r=col.r;
	vert.g=col.g;
	vert.b=col.b;
	vert.a=col.a;
}
void Beam::Init (const Transformation & trans, const weapon_info &cln , void * own)  {
  //Matrix m;
  CollideInfo.object.b = NULL;
  CollideInfo.type = LineCollide::BEAM;
  if (vlist)
    delete vlist;
  local_transformation = trans;//location on ship
  //  cumalative_transformation =trans; 
  //  trans.to_matrix (cumalative_transformation_matrix);
  speed = cln.Speed;
  texturespeed = cln.PulseSpeed;
  range = cln.Range;
  radialspeed = cln.RadialSpeed;
  thickness = cln.Radius;
  stability = cln.Stability;
  rangepenalty=cln.Longrange;
  damagerate = cln.Damage;
  phasedamage= cln.PhaseDamage;
  texturestretch=cln.TextureStretch;
  refiretime = 0;
  refire = cln.Refire;
  Col.r = cln.r;
  Col.g = cln.g;
  Col.b = cln.b;
  Col.a=cln.a;
  impact= ALIVE;
  owner = own;
  numframes=0;

  lastlength=0;
  curlength = SIMULATION_ATOM*speed;
  lastthick=0;
  curthick = SIMULATION_ATOM*radialspeed;
  GFXColorVertex beam[48];
  memset(beam,0,sizeof(GFXColorVertex)*32);
  GFXColorVertex * calah=beam;
  SetColorToVertex(calah[0]);
  SetColorToVertex(calah[1]);  
  SetColorToVertex(calah[2],Col);
  SetColorToVertex(calah[3],Col);
  SetColorToVertex(calah[4],Col);
  SetColorToVertex(calah[5],Col);    
  SetColorToVertex(calah[6]);
  SetColorToVertex(calah[7]);
  SetColorToVertex(calah[8]);
  SetColorToVertex(calah[9]);
  SetColorToVertex(calah[10]);
  SetColorToVertex(calah[11],Col);
  SetColorToVertex(calah[12]);
  SetColorToVertex(calah[13]);
  SetColorToVertex(calah[14]);
  SetColorToVertex(calah[15],Col);
  SetColorToVertex(calah[16]);
  SetColorToVertex(calah[17]);
  SetColorToVertex(calah[18]);
  SetColorToVertex(calah[19],Col);
  SetColorToVertex(calah[20]);
  SetColorToVertex(calah[21]);
  SetColorToVertex(calah[22]);
  SetColorToVertex(calah[23],Col);  
  ScaleByAlpha (calah[2],Col.a);
  ScaleByAlpha (calah[3],Col.a);
  ScaleByAlpha (calah[4],Col.a);
  ScaleByAlpha (calah[5],Col.a);
  ScaleByAlpha (calah[11],Col.a);
  ScaleByAlpha (calah[15],Col.a);		
  ScaleByAlpha (calah[19],Col.a);
  ScaleByAlpha (calah[23],Col.a);		
  //since mode is ONE,ONE
  memcpy (&calah[24],&calah[0],sizeof(GFXColorVertex)*24);
  
  vlist = new GFXVertexList (GFXQUAD,48,calah,48,true);//mutable color contained list
#ifdef PERBOLTSOUND
  AUDStartPlaying (sound);
#endif
}

void Beam::RecalculateVertices() {
  GFXColorVertex * beam = (vlist->BeginMutate(0))->colors;
  static float fadelocation = XMLSupport::parse_float (vs_config->getVariable ("graphics","BeamFadeoutLength",".8"));
  static float hitfadelocation=XMLSupport::parse_float (vs_config->getVariable ("graphics","BeamFadeoutHitLength",".95"));
  const float fadeinlength = 4;
  
  float leftex = -texturespeed*(numframes*SIMULATION_ATOM+interpolation_blend_factor*SIMULATION_ATOM);
  float righttex = leftex+texturestretch*curlength/curthick;//how long compared to how wide!
  float len = (impact==ALIVE)?
	  (curlength<range?
	          curlength - speed*SIMULATION_ATOM*(1-interpolation_blend_factor):
	          range)
   :
	  curlength;
  float fadelen = (impact==ALIVE)?
	  len*fadelocation
	  :
	  len*hitfadelocation;
//	  (range*fadelocation>curlength?  // Not sure what this shit means
//	                 curlength:
//	                 range*fadelocation);
  float fadetex = leftex + (righttex-leftex)*fadelocation;
  const float touchtex = leftex-fadeinlength*.5*texturestretch;
  float thick = curthick!=thickness?curthick-radialspeed*SIMULATION_ATOM*(1-interpolation_blend_factor):thickness;
  const float invfadelen=thick*fadeinlength;
  int a=0;
#define V(xx,yy,zz,ss,tt) { beam[a].x = xx; beam[a].y = yy; beam[a].z = zz; beam[a].s=ss; beam[a].t=tt;a++; }

  V(0,thick,invfadelen,leftex,1);
  V(0,thick,fadelen,fadetex,1);
  V(0,0,fadelen,fadetex,.5);
  V(0,0,invfadelen,leftex,.5);
  V(0,0,invfadelen,leftex,.5);
  V(0,0,fadelen,fadetex,.5);
  V(0,-thick,fadelen,fadetex,0);
  V(0,-thick,invfadelen,leftex,0);
///fade out
  V(0,thick,fadelen,fadetex,1);
  V(0,thick,len,righttex,1);
  V(0,0,len,righttex,.5);
  V(0,0,fadelen,fadetex,.5);

  V(0,-thick,fadelen,fadetex,0);
  V(0,-thick,len,righttex,0);
  V(0,0,len,righttex,.5);
  V(0,0,fadelen,fadetex,.5);
///fade in
  V(0,thick,invfadelen,leftex,1);
  V(0,thick,0,touchtex,1);
  V(0,0,0,touchtex,.5);
  V(0,0,invfadelen,leftex,.5);

  V(0,-thick,invfadelen,leftex,0);
  V(0,-thick,0,touchtex,0);
  V(0,0,0,touchtex,.5);
  V(0,0,invfadelen,leftex,.5);




#undef V//reverse the notation for the rest of the identical vertices
#define QV(yy,xx,zz,ss,tt) { beam[a].x = xx; beam[a].y = yy; beam[a].z = zz; beam[a].s=ss; beam[a].t=tt;a++; }

  QV(0,thick,invfadelen,leftex,1);
  QV(0,thick,fadelen,fadetex,1);
  QV(0,0,fadelen,fadetex,.5);
  QV(0,0,invfadelen,leftex,.5);
  QV(0,0,invfadelen,leftex,.5);
  QV(0,0,fadelen,fadetex,.5);
  QV(0,-thick,fadelen,fadetex,0);
  QV(0,-thick,invfadelen,leftex,0);
///fade out
  QV(0,thick,fadelen,fadetex,1);
  QV(0,thick,len,righttex,1);
  QV(0,0,len,righttex,.5);
  QV(0,0,fadelen,fadetex,.5);

  QV(0,-thick,fadelen,fadetex,0);
  QV(0,-thick,len,righttex,0);
  QV(0,0,len,righttex,.5);
  QV(0,0,fadelen,fadetex,.5);
//fadein
  QV(0,thick,invfadelen,leftex,1);
  QV(0,thick,0,touchtex,1);
  QV(0,0,0,touchtex,.5);
  QV(0,0,invfadelen,leftex,.5);

  QV(0,-thick,invfadelen,leftex,0);
  QV(0,-thick,0,touchtex,0);
  QV(0,0,0,touchtex,.5);
  QV(0,0,invfadelen,leftex,.5);


#undef QV
  vlist->EndMutate();
}

void Beam::RemoveFromSystem(bool eradicate) {
  //beams not in table any more
#if 0
  if (CollideInfo.object.b!=NULL
#ifndef SAFE_COLLIDE_DEBUG
#ifndef UNSAFE_COLLIDE_RELEASE
      &&!eradicate
#endif
#endif
) {
    KillCollideTable (&CollideInfo);
    CollideInfo.object.b = NULL;
  }
#ifndef UNSAFE_COLLIDE_RELEASE
  if (eradicate) {
#ifdef SAFE_COLLIDE_DEBUG
    if (
#endif
	EradicateCollideTable (&CollideInfo)
#ifdef SAFE_COLLIDE_DEBUG 
	) {
      fprintf (stderr,"RECOVERED from (formerly) fatal, currently nonfatal error with beam deletion\n");      
    }
#else
    ;
#endif
    for (unsigned int i=0;i<_Universe->star_system.size();i++) {
      _Universe->pushActiveStarSystem(_Universe->star_system[i]);
    
    if (EradicateCollideTable (&CollideInfo)) {
      fprintf (stderr,"VERY BAD ERROR FATAL! 0x%x",this);
    }
    _Universe->popActiveStarSystem();
    }
    CollideInfo.object.b = NULL;

  }
#endif
#endif
}
void Beam::UpdatePhysics(const Transformation &trans, const Matrix &m, Unit * targ, float tracking_cone, Unit * targetToCollideWith) {
  curlength += SIMULATION_ATOM*speed;
  if (curlength<0) {
    curlength=0;
  }
  if (curthick ==0) {
    //#ifdef PERBOLTSOUND
    //#endif
    if (AUDIsPlaying(sound)&&refiretime>=SIMULATION_ATOM)
      AUDStopPlaying (sound);
    refiretime +=SIMULATION_ATOM;
 
    return;
  }
  if (stability&&numframes*SIMULATION_ATOM>stability)
    impact|=UNSTABLE;

  numframes++;
  Matrix cumulative_transformation_matrix;
  Transformation cumulative_transformation = local_transformation;
  cumulative_transformation.Compose(trans, m);
  cumulative_transformation.to_matrix(cumulative_transformation_matrix);
  AdjustMatrix (cumulative_transformation_matrix,Vector(0,0,0),targ,speed,false,tracking_cone);
  //to help check for crashing.
  center = cumulative_transformation.position;
  direction = TransformNormal (cumulative_transformation_matrix,Vector(0,0,1));
#ifndef PERFRAMESOUND
  AUDAdjustSound (sound,cumulative_transformation.position,speed*cumulative_transformation_matrix.getR());
#endif
  
  curthick+=(impact&UNSTABLE)?-radialspeed*SIMULATION_ATOM:radialspeed*SIMULATION_ATOM;
  if (curthick > thickness)
    curthick = thickness;


  if (curthick<=0) {

    curthick =0;//die die die
#ifdef BEAMCOLQ
    RemoveFromSystem(false);
#endif    
  } else {

    CollideHuge(CollideInfo,listen_to_owner?targetToCollideWith:NULL);
    
    if (!(curlength<range&&curlength>0)) {//if curlength just happens to be nan
      if (curlength>range)
	curlength=range;
      else
	curlength=0;
    }
    QVector tmpvec (center + direction.Cast().Scale(curlength));
    QVector tmpMini = center.Min(tmpvec);


    tmpvec = center.Max (tmpvec);
#ifdef BEAMCOLQ
        if (TableLocationChanged (CollideInfo,tmpMini,tmpvec)||(curthick>0&&CollideInfo.object.b==NULL)) {
	  RemoveFromSystem(false);
#endif
      CollideInfo.object.b = this;
      CollideInfo.hhuge=(((CollideInfo.Maxi.i-CollideInfo.Mini.i)/coltableacc)*((CollideInfo.Maxi.j-CollideInfo.Mini.j)/coltableacc)*(CollideInfo.Maxi.k-CollideInfo.Mini.k)/coltableacc>tablehuge);
      CollideInfo.Mini= tmpMini;
      CollideInfo.Maxi= tmpvec;
#ifdef BEAMCOLQ
      AddCollideQueue (CollideInfo);
    } else {
      CollideInfo.Mini= tmpMini;
      CollideInfo.Maxi= tmpvec;
    }
#endif
  }
  //Check if collide...that'll change max beam length REAL quick
}
extern Cargo * GetMasterPartList (const char *);
bool Beam::Collide (Unit * target) {
  if (this==NULL||target==NULL){
    fprintf (stderr,"Recovering from nonfatal beam error when beam inactive\n");
    return false;
  }
  float distance;
  Vector normal;//apply shields
  QVector end (center+direction.Cast().Scale(curlength));
  enum clsptr type = target->isUnit();
  if (target==owner||type==NEBULAPTR||type==ASTEROIDPTR) {
    static bool collideroids = XMLSupport::parse_bool(vs_config->getVariable("physics","AsteroidWeaponCollision","false"));
      if (type!=ASTEROIDPTR||(!collideroids)) {
	return false;
      }
  }
  

  Unit * colidee;
  if ((colidee = target->queryBSP(center,end,normal,distance))) { 
  //  QVector pos;
  //  double dis=distance;
  //  if ((colidee = target->BeamInsideCollideTree(center,end,pos,normal,dis))) { 
  //    distance=dis;
    curlength = distance;
    impact|=IMPACT;
    
    GFXColor coltmp (Col);
    /*
    coltmp.r+=.5;
    coltmp.g+=.5;
    coltmp.b+=.5;
    if (coltmp.r>1)coltmp.r=1;
    if (coltmp.g>1)coltmp.g=1;
    if (coltmp.b>1)coltmp.b=1;
    */
    float tmp=(curlength/range); 
    float appldam = (damagerate*SIMULATION_ATOM*curthick/thickness)*((1-tmp)+tmp*rangepenalty);
    float phasdam = (phasedamage*SIMULATION_ATOM*curthick/thickness)*((1-tmp)+tmp*rangepenalty);
    if ((appldam<0&&phasdam>0)||(appldam>0&&phasdam<0)) {
      //tractor beam!
      target->ApplyForce (direction*appldam);
      if ((center-target->Position()).Magnitude()<fabs(phasdam)) {

	un_iter ui= _Universe->activeStarSystem()->getUnitList().createIterator();
	Unit *un;
	for (;(un=*ui)!=NULL;++ui) {
	  if (((void *)un)==owner) {
	    static float nbig = .1;
	    if (target->faction==FactionUtil::GetFaction("upgrades")||un->rSize()*nbig>target->rSize()) {
	      //we have our man!
	      //lets add our cargo to him
	      Cargo *c = GetMasterPartList (target->name.c_str());
	      Cargo tmp;
	      if (c==NULL) {
		c=&tmp;
		tmp.content="spacejunk";
		tmp.category="misc";
		tmp.price=200;
		tmp.quantity=1;
		tmp.mass=.001;
		tmp.volume=1;
	      }
	      if (c!=NULL) {
		Cargo adder = *c;
		adder.quantity=1;
		if (un->CanAddCargo(adder)) {
		  un->AddCargo(adder);
		  target->Kill();
		}
	      }
	    }
	  }
	}
      }
    }else {
//      if (appldam>0==phasdam>0||applydam==0||phasedam==0) {
	target->ApplyDamage (center.Cast()+direction*curlength,normal,appldam,colidee,coltmp,(Unit *)owner,phasdam);
	//}else if (damagerate<0||phasedamage<0) {
	  //target->leach (1,phasedamage<0?-phasedamage*SIMULATION_ATOM:1,damagerate<0?-damagerate*SIMULATION_ATOM:1);
      //}
    }
    return true;
    
  }
  
  return false;
}

