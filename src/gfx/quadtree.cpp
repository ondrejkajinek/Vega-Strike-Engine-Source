/**
 *
 * Data structures for quadtree terrain storage.
 * This code may be freely modified and redistributed.  I make no
 * warrantees about it; use at your own risk.  If you do incorporate
 * this code into a project, I'd appreciate a mention in the credits.
 * Thatcher Ulrich <tu@tulrich.com>
 *
 * Rewritten and adapted to Vegastrike by Daniel Horn
 */


//#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <math.h>
#include "quadtree.h"
#include "gfxlib.h"




//
// quadsquare functions.
//

quadsquare::quadsquare(quadcornerdata* pcd) {
// Constructor.

	pcd->Square = this;
	
	// Set static to true if/when this node contains real data, and
	// not just interpolated values.  When static == false, a node
	// can be deleted by the Update() function if none of its
	// vertices or children are enabled.
	Static = false;
	
	int	i;
	for (i = 0; i < 4; i++) {
		Child[i] = NULL;
	}

	EnabledFlags = 0;
	
	for (i = 0; i < 2; i++) {
		SubEnabledCount[i] = 0;
	}
	
	// Set default vertex positions by interpolating from given corners.
	// Just bilinear interpolation.
	Vertex[0].Y = 0.25 * (pcd->Verts[0].Y + pcd->Verts[1].Y + pcd->Verts[2].Y + pcd->Verts[3].Y);
	Vertex[1].Y = 0.5 * (pcd->Verts[3].Y + pcd->Verts[0].Y);
	Vertex[2].Y = 0.5 * (pcd->Verts[0].Y + pcd->Verts[1].Y);
	Vertex[3].Y = 0.5 * (pcd->Verts[1].Y + pcd->Verts[2].Y);
	Vertex[4].Y = 0.5 * (pcd->Verts[2].Y + pcd->Verts[3].Y);

	for (i = 0; i < 2; i++) {
		Error[i] = 0;
	}
	for (i = 0; i < 4; i++) {
		Error[i+2] = (unsigned short) (fabs((Vertex[0].Y + pcd->Verts[i].Y) - (Vertex[i+1].Y + Vertex[((i+1)&3) + 1].Y)) * 0.25);
	}

	// Compute MinY/MaxY based on corner verts.
	MinY = MaxY = (unsigned short) pcd->Verts[0].Y;
	for (i = 1; i < 4; i++) {
		float	y = pcd->Verts[i].Y;
		if (y < MinY) MinY = (unsigned short)y;
		if (y > MaxY) MaxY = (unsigned short)y;
	}
	
	// Initialize colors by interpolating from corners.
	Vertex[0].Norm = (pcd->Verts[0].Norm + pcd->Verts[1].Norm + pcd->Verts[2].Norm + pcd->Verts[3].Norm).Normalize();
	Vertex[1].Norm = (pcd->Verts[3].Norm + pcd->Verts[0].Norm).Normalize();
	Vertex[2].Norm = (pcd->Verts[0].Norm + pcd->Verts[1].Norm).Normalize();
	Vertex[3].Norm = (pcd->Verts[1].Norm + pcd->Verts[2].Norm).Normalize();
	Vertex[4].Norm = (pcd->Verts[2].Norm + pcd->Verts[3].Norm).Normalize();
}


quadsquare::~quadsquare() {
	// Recursively delete sub-trees.
	int	i;
	for (i = 0; i < 4; i++) {
		if (Child[i]) delete Child[i];
		Child[i] = NULL;
	}
}

  /**
   * Sets this node's static flag to true.  If static == true, then the
   * node or its children is considered to contain significant height data
   * and shouldn't be deleted.
   */
void	quadsquare::SetStatic(const quadcornerdata& cd) {
	if (Static == false) {
		Static = true;
		
		// Propagate static status to ancestor nodes.
		if (cd.Parent && cd.Parent->Square) {
			cd.Parent->Square->SetStatic(*cd.Parent);
		}
	}
}


int	quadsquare::CountNodes()
// Debugging function.  Counts the number of nodes in this subtree.
{
	int	count = 1;	// Count ourself.

	// Count descendants.
	for (int i = 0; i < 4; i++) {
		if (Child[i]) count += Child[i]->CountNodes();
	}

	return count;
}
/**
 * Returns the height of the heightfield at the specified x,z coordinates.
 * Can be used for collision detection
 */
float	quadsquare::GetHeight(const quadcornerdata& cd, float x, float z)

{
	int	half = 1 << cd.Level;

	float	lx = (x - cd.xorg) / float(half);
	float	lz = (z - cd.zorg) / float(half);

	int	ix = (int)floor(lx);
	int	iz = (int)floor(lz);

	// Clamp.
	if (ix < 0) ix = 0;
	if (ix > 1) ix = 1;
	if (iz < 0) iz = 0;
	if (iz > 1) iz = 1;

	int	index = ix ^ (iz ^ 1) + (iz << 1);
	if (Child[index] && Child[index]->Static) {
		// Pass the query down to the child which contains it.
		quadcornerdata	q;
		SetupCornerData(&q, cd, index);
		return Child[index]->GetHeight(q, x, z);
	}

	// Bilinear interpolation.
	lx -= ix;
	if (lx < 0) lx = 0;
	if (lx > 1) lx = 1;
	
	lz -= iz;
	if (lx < 0) lz = 0;
	if (lz > 1) lz = 1;

	float	s00, s01, s10, s11;
	switch (index) {
	default:
	case 0:
		s00 = Vertex[2].Y;
		s01 = cd.Verts[0].Y;
		s10 = Vertex[0].Y;
		s11 = Vertex[1].Y;
		break;
	case 1:
		s00 = cd.Verts[1].Y;
		s01 = Vertex[2].Y;
		s10 = Vertex[3].Y;
		s11 = Vertex[0].Y;
		break;
	case 2:
		s00 = Vertex[3].Y;
		s01 = Vertex[0].Y;
		s10 = cd.Verts[2].Y;
		s11 = Vertex[4].Y;
		break;
	case 3:
		s00 = Vertex[0].Y;
		s01 = Vertex[1].Y;
		s10 = Vertex[4].Y;
		s11 = cd.Verts[3].Y;
		break;
	}

	return (s00 * (1-lx) + s01 * lx) * (1 - lz) + (s10 * (1-lx) + s11 * lx) * lz;
}


quadsquare*	quadsquare::GetNeighbor(int dir, const quadcornerdata& cd)
// Traverses the tree in search of the quadsquare neighboring this square to the
// specified direction.  0-3 --> { E, N, W, S }.
// Returns NULL if the neighbor is outside the bounds of the tree.
{
	// If we don't have a parent, then we don't have a neighbor.
	// (Actually, we could have inter-tree connectivity at this level
	// for connecting separate trees together.)
	if (cd.Parent == 0) return 0;
	
	// Find the parent and the child-index of the square we want to locate or create.
	quadsquare*	p = 0;
	
	int	index = cd.ChildIndex ^ 1 ^ ((dir & 1) << 1);
	bool	SameParent = ((dir - cd.ChildIndex) & 2) ? true : false;
	
	if (SameParent) {
		p = cd.Parent->Square;
	} else {
		p = cd.Parent->Square->GetNeighbor(dir, *cd.Parent);
		
		if (p == 0) return 0;
	}
	
	quadsquare*	n = p->Child[index];
	
	return n;
}


Vector	MakeLightness(float xslope, float zslope) {
  return (Vector(xslope, 1, zslope)).Normalize();
}

  /**
   * Recomputes the error values for this tree.  Returns the
   * max error.
   * Also updates MinY & MaxY.
   * Also computes quick & dirty vertex lighting for the demo.
   */
float quadsquare::RecomputeErrorAndLighting(const quadcornerdata& cd) {
	int	i;
	
	// Measure error of center and edge vertices.
	float	maxerror = 0;

	// Compute error of center vert.
	float	e;
	if (cd.ChildIndex & 1) {
		e = fabs(Vertex[0].Y - (cd.Verts[1].Y + cd.Verts[3].Y) * 0.5);
	} else {
		e = fabs(Vertex[0].Y - (cd.Verts[0].Y + cd.Verts[2].Y) * 0.5);
	}
	if (e > maxerror) maxerror = e;

	// Initial min/max.
	MaxY = (unsigned short)Vertex[0].Y;
	MinY = (unsigned short)Vertex[0].Y;

	// Check min/max of corners.
	for (i = 0; i < 4; i++) {
		float	y = cd.Verts[i].Y;
		if (y < MinY) MinY = (unsigned short)y;
		if (y > MaxY) MaxY = (unsigned short)y;
	}
	
	// Edge verts.
	e = fabs(Vertex[1].Y - (cd.Verts[0].Y + cd.Verts[3].Y) * 0.5);
	if (e > maxerror) maxerror = e;
	Error[0] = (unsigned short)e;
	
	e = fabs(Vertex[4].Y - (cd.Verts[2].Y + cd.Verts[3].Y) * 0.5);
	if (e > maxerror) maxerror = e;
	Error[1] = (unsigned short)e;

	// Min/max of edge verts.
	for (i = 0; i < 4; i++) {
		float	y = Vertex[1 + i].Y;
		if (y < MinY) MinY = (unsigned short)y;
		if (y > MaxY) MaxY = (unsigned short)y;
	}
	
	// Check child squares.
	for (i = 0; i < 4; i++) {
		quadcornerdata	q;
		if (Child[i]) {
			SetupCornerData(&q, cd, i);
			Error[i+2] = (unsigned short)Child[i]->RecomputeErrorAndLighting(q);

			if (Child[i]->MinY < MinY) MinY = Child[i]->MinY;
			if (Child[i]->MaxY > MaxY) MaxY = Child[i]->MaxY;
		} else {
			// Compute difference between bilinear average at child center, and diagonal edge approximation.
			Error[i+2] = (unsigned short)(fabs((Vertex[0].Y + cd.Verts[i].Y) - (Vertex[i+1].Y + Vertex[((i+1)&3) + 1].Y)) * 0.25);
		}
		if (Error[i+2] > maxerror) maxerror = Error[i+2];
	}


	//
	// Compute quickie demo lighting.
	//

	float	OneOverSize = 1.0 / (2 << cd.Level);
	Vertex[0].Norm = MakeLightness((Vertex[1].Y - Vertex[3].Y) * OneOverSize,
				       (Vertex[4].Y - Vertex[2].Y) * OneOverSize);

	float	v;
	quadsquare*	s = GetNeighbor(0, cd);
	if (s) v = s->Vertex[0].Y; else v = Vertex[1].Y;
	Vertex[1].Norm = MakeLightness((v - Vertex[0].Y) * OneOverSize,
				       (cd.Verts[3].Y - cd.Verts[0].Y) * OneOverSize);
	
	s = GetNeighbor(1, cd);
	if (s) v = s->Vertex[0].Y; else v = Vertex[2].Y;
	Vertex[2].Norm = MakeLightness((cd.Verts[0].Y - cd.Verts[1].Y) * OneOverSize,
				       (Vertex[0].Y - v) * OneOverSize);
	
	s = GetNeighbor(2, cd);
	if (s) v = s->Vertex[0].Y; else v = Vertex[3].Y;
	Vertex[3].Norm = MakeLightness((Vertex[0].Y - v) * OneOverSize,
				       (cd.Verts[2].Y - cd.Verts[1].Y) * OneOverSize);
	
	s = GetNeighbor(3, cd);
	if (s) v = s->Vertex[0].Y; else v = Vertex[4].Y;
	Vertex[4].Norm = MakeLightness((cd.Verts[3].Y - cd.Verts[2].Y) * OneOverSize,
				       (v - Vertex[0].Y) * OneOverSize);
	

	// The error, MinY/MaxY, and lighting values for this node and descendants are correct now.
	Dirty = false;
	
	return maxerror;
}


void	quadsquare::ResetTree()
// Clear all enabled flags, and delete all non-static child nodes.
{
	int	i;
	for (i = 0; i < 4; i++) {
		if (Child[i]) {
			Child[i]->ResetTree();
			if (Child[i]->Static == false) {
				delete Child[i];
				Child[i] = 0;
			}
		}
	}
	EnabledFlags = 0;
	SubEnabledCount[0] = 0;
	SubEnabledCount[1] = 0;
	Dirty = true;
}


void	quadsquare::StaticCullData(const quadcornerdata& cd, float ThresholdDetail)
// Examine the tree and remove nodes which don't contain necessary
// detail.  Necessary detail is defined as vertex data with a
// edge-length to height ratio less than ThresholdDetail.
{
	// First, clean non-static nodes out of the tree.
	ResetTree();

	// Make sure error values are up-to-date.
	if (Dirty) RecomputeErrorAndLighting(cd);
	
	// Recursively check all the nodes and do necessary removal.
	// We must start at the bottom of the tree, and do one level of
	// the tree at a time, to ensure the dependencies are accounted
	// for properly.
	int	level;
	for (level = 0; level < 15; level++) {
		StaticCullAux(cd, ThresholdDetail, level);
	}
}


void	quadsquare::StaticCullAux(const quadcornerdata& cd, float ThresholdDetail, int TargetLevel)
// Check this node and its descendents, and remove nodes which don't contain
// necessary detail.
{
	int	i, j;
	quadcornerdata	q;

	if (cd.Level > TargetLevel) {
		// Just recurse to child nodes.
		for (j = 0; j < 4; j++) {
			if (j < 2) i = 1 - j;
			else i = j;

			if (Child[i]) {
				SetupCornerData(&q, cd, i);
				Child[i]->StaticCullAux(q, ThresholdDetail, TargetLevel);
			}
		}
		return;
	}

	// We're at the target level.  Check this node to see if it's OK to delete it.

	// Check edge vertices to see if they're necessary.
	float	size = 2 << cd.Level;	// Edge length.
	if (Child[0] == NULL && Child[3] == NULL && Error[0] * ThresholdDetail < size) {
		quadsquare*	s = GetNeighbor(0, cd);
		if (s == NULL || (s->Child[1] == NULL && s->Child[2] == NULL)) {

			// Force vertex height to the edge value.
			float	y = (cd.Verts[0].Y + cd.Verts[3].Y) * 0.5;
			Vertex[1].Y = y;
			Error[0] = 0;
			
			// Force alias vertex to match.
			if (s) s->Vertex[3].Y = y;
			
			Dirty = true;
		}
	}

	if (Child[2] == NULL && Child[3] == NULL && Error[1] * ThresholdDetail < size) {
		quadsquare*	s = GetNeighbor(3, cd);
		if (s == NULL || (s->Child[0] == NULL && s->Child[1] == NULL)) {
			float	y = (cd.Verts[2].Y + cd.Verts[3].Y) * 0.5;
			Vertex[4].Y = y;
			Error[1] = 0;
			
			if (s) s->Vertex[2].Y = y;
			
			Dirty = true;
		}
	}

	// See if we have child nodes.
	bool	StaticChildren = false;
	for (i = 0; i < 4; i++) {
		if (Child[i]) {
			StaticChildren = true;
			if (Child[i]->Dirty) Dirty = true;
		}
	}

	// If we have no children and no necessary edges, then see if we can delete ourself.
	if (StaticChildren == false && cd.Parent != NULL) {
		bool	NecessaryEdges = false;
		for (i = 0; i < 4; i++) {
			// See if vertex deviates from edge between corners.
			float	diff = fabs(Vertex[i+1].Y - (cd.Verts[i].Y + cd.Verts[(i+3)&3].Y) * 0.5);
			if (diff > 0.00001) {
				NecessaryEdges = true;
			}
		}

		if (!NecessaryEdges) {
			size *= 1.414213562;	// sqrt(2), because diagonal is longer than side.
			if (cd.Parent->Square->Error[2 + cd.ChildIndex] * ThresholdDetail < size) {
				delete cd.Parent->Square->Child[cd.ChildIndex];	// Delete this.
				cd.Parent->Square->Child[cd.ChildIndex] = 0;	// Clear the pointer.
			}
		}
	}
}


int	MaxCreateDepth = 0;


void	quadsquare::EnableEdgeVertex(int index, bool IncrementCount, const quadcornerdata& cd)
// Enable the specified edge vertex.  Indices go { e, n, w, s }.
// Increments the appropriate reference-count if IncrementCount is true.
{
	if ((EnabledFlags & (1 << index)) && IncrementCount == false) return;
	
	static const int	Inc[4] = { 1, 0, 0, 8 };

	// Turn on flag and deal with reference count.
	EnabledFlags |= 1 << index;
	if (IncrementCount == true && (index == 0 || index == 3)) {
		SubEnabledCount[index & 1]++;
	}

	// Now we need to enable the opposite edge vertex of the adjacent square (i.e. the alias vertex).

	// This is a little tricky, since the desired neighbor node may not exist, in which
	// case we have to create it, in order to prevent cracks.  Creating it may in turn cause
	// further edge vertices to be enabled, propagating updates through the tree.

	// The sticking point is the quadcornerdata list, which
	// conceptually is just a linked list of activation structures.
	// In this function, however, we will introduce branching into
	// the "list", making it in actuality a tree.  This is all kind
	// of obscure and hard to explain in words, but basically what
	// it means is that our implementation has to be properly
	// recursive.

	// Travel upwards through the tree, looking for the parent in common with our desired neighbor.
	// Remember the path through the tree, so we can travel down the complementary path to get to the neighbor.
	quadsquare*	p = this;
	const quadcornerdata*	pcd = &cd;
	int	ct = 0;
	int	stack[32];
	for (;;) {
		int	ci = pcd->ChildIndex;

		if (pcd->Parent == NULL || pcd->Parent->Square == NULL) {
			// Neighbor doesn't exist (it's outside the tree), so there's no alias vertex to enable.
			return;
		}
		p = pcd->Parent->Square;
		pcd = pcd->Parent;

		bool	SameParent = ((index - ci) & 2) ? true : false;
		
		ci = ci ^ 1 ^ ((index & 1) << 1);	// Child index of neighbor node.

		stack[ct] = ci;
		ct++;
		
		if (SameParent) break;
	}

	// Get a pointer to our neighbor (create if necessary), by walking down
	// the quadtree from our shared ancestor.
	p = p->EnableDescendant(ct, stack, *pcd);
	
	// Finally: enable the vertex on the opposite edge of our neighbor, the alias of the original vertex.
	index ^= 2;
	p->EnabledFlags |= (1 << index);
	if (IncrementCount == true && (index == 0 || index == 3)) {
		p->SubEnabledCount[index & 1]++;
	}
}


quadsquare*	quadsquare::EnableDescendant(int count, int path[], const quadcornerdata& cd)
// This function enables the descendant node 'count' generations below
// us, located by following the list of child indices in path[].
// Creates the node if necessary, and returns a pointer to it.
{
	count--;
	int	ChildIndex = path[count];

	if ((EnabledFlags & (16 << ChildIndex)) == 0) {
		EnableChild(ChildIndex, cd);
	}
	
	if (count > 0) {
		quadcornerdata	q;
		SetupCornerData(&q, cd, ChildIndex);
		return Child[ChildIndex]->EnableDescendant(count, path, q);
	} else {
		return Child[ChildIndex];
	}
}


void	quadsquare::CreateChild(int index, const quadcornerdata& cd)
// Creates a child square at the specified index.
{
	if (Child[index] == 0) {
		quadcornerdata	q;
		SetupCornerData(&q, cd, index);
		
		Child[index] = new quadsquare(&q);
	}
}


void	quadsquare::EnableChild(int index, const quadcornerdata& cd)
// Enable the indexed child node.  { ne, nw, sw, se }
// Causes dependent edge vertices to be enabled.
{
//	if (Enabled[index + 4] == false) {
	if ((EnabledFlags & (16 << index)) == 0) {
//		Enabled[index + 4] = true;
		EnabledFlags |= (16 << index);
		EnableEdgeVertex(index, true, cd);
		EnableEdgeVertex((index + 1) & 3, true, cd);
		
		if (Child[index] == 0) {
			CreateChild(index, cd);
		}
	}
}




void	quadsquare::NotifyChildDisable(const quadcornerdata& cd, int index)
// Marks the indexed child quadrant as disabled.  Deletes the child node
// if it isn't static.
{
	// Clear enabled flag for the child.
	EnabledFlags &= ~(16 << index);
	
	// Update child enabled counts for the affected edge verts.
	quadsquare*	s;
	
	if (index & 2) s = this;
	else s = GetNeighbor(1, cd);
	if (s) {
		s->SubEnabledCount[1]--;
	}
	
	if (index == 1 || index == 2) s = GetNeighbor(2, cd);
	else s = this;
	if (s) {
		s->SubEnabledCount[0]--;
	}
	
	if (Child[index]->Static == false) {
		delete Child[index];
		Child[index] = 0;

	}
}


static float	DetailThreshold = 100;


bool	VertexTest(float x, float y, float z, float error, const Vector & Viewer)
// Returns true if the vertex at (x,z) with the given world-space error between
// its interpolated location and its true location, should be enabled, given that
// the viewpoint is located at Viewer[].
{
	float	dx = fabs(x - Viewer.i);
	float	dy = fabs(y - Viewer.j);
	float	dz = fabs(z - Viewer.k);
	float	d = dx;
	if (dy > d) d = dy;
	if (dz > d) d = dz;

	return (error * DetailThreshold) > d;
}


bool	BoxTest(float x, float z, float size, float miny, float maxy, float error, const Vector & Viewer)
// Returns true if any vertex within the specified box (origin at x,z,
// edges of length size) with the given error value could be enabled
// based on the given viewer location.
{
	// Find the minimum distance to the box.
	float	half = size * 0.5;
	float	dx = fabs(x + half - Viewer.i) - half;
	float	dy = fabs((miny + maxy) * 0.5 - Viewer.j) - (maxy - miny) * 0.5;
	float	dz = fabs(z + half - Viewer.k) - half;
	float	d = dx;
	if (dy > d) d = dy;
	if (dz > d) d = dz;

	return (error * DetailThreshold) > d;
}


//const float	VERTICAL_SCALE = 1.0 / 8.0;
const float	VERTICAL_SCALE = 1.0;


void	quadsquare::Update(const quadcornerdata& cd, const Vector & ViewerLocation, float Detail)
// Refresh the vertex enabled states in the tree, according to the
// location of the viewer.  May force creation or deletion of qsquares
// in areas which need to be interpolated.
{
	DetailThreshold = Detail * VERTICAL_SCALE;
	
	UpdateAux(cd, ViewerLocation, 0);
}


void	quadsquare::UpdateAux(const quadcornerdata& cd, const Vector & ViewerLocation, float CenterError)
// Does the actual work of updating enabled states and tree growing/shrinking.
{
	
	// Make sure error values are current.
	if (Dirty) {
		RecomputeErrorAndLighting(cd);
	}

	int	half = 1 << cd.Level;
	int	whole = half << 1;

	// See about enabling child verts.
	if ((EnabledFlags & 1) == 0 && VertexTest(cd.xorg + whole, Vertex[1].Y, cd.zorg + half, Error[0], ViewerLocation) == true) EnableEdgeVertex(0, false, cd);	// East vert.
	if ((EnabledFlags & 8) == 0 && VertexTest(cd.xorg + half, Vertex[4].Y, cd.zorg + whole, Error[1], ViewerLocation) == true) EnableEdgeVertex(3, false, cd);	// South vert.
	if (cd.Level > 0) {
		if ((EnabledFlags & 32) == 0) {
			if (BoxTest(cd.xorg, cd.zorg, half, MinY, MaxY, Error[3], ViewerLocation) == true) EnableChild(1, cd);	// nw child.er
		}
		if ((EnabledFlags & 16) == 0) {
			if (BoxTest(cd.xorg + half, cd.zorg, half, MinY, MaxY, Error[2], ViewerLocation) == true) EnableChild(0, cd);	// ne child.
		}
		if ((EnabledFlags & 64) == 0) {
			if (BoxTest(cd.xorg, cd.zorg + half, half, MinY, MaxY, Error[4], ViewerLocation) == true) EnableChild(2, cd);	// sw child.
		}
		if ((EnabledFlags & 128) == 0) {
			if (BoxTest(cd.xorg + half, cd.zorg + half, half, MinY, MaxY, Error[5], ViewerLocation) == true) EnableChild(3, cd);	// se child.
		}
		
		// Recurse into child quadrants as necessary.
		quadcornerdata	q;
		
		if (EnabledFlags & 32) {
			SetupCornerData(&q, cd, 1);
			Child[1]->UpdateAux(q, ViewerLocation, Error[3]);
		}
		if (EnabledFlags & 16) {
			SetupCornerData(&q, cd, 0);
			Child[0]->UpdateAux(q, ViewerLocation, Error[2]);
		}
		if (EnabledFlags & 64) {
			SetupCornerData(&q, cd, 2);
			Child[2]->UpdateAux(q, ViewerLocation, Error[4]);
		}
		if (EnabledFlags & 128) {
			SetupCornerData(&q, cd, 3);
			Child[3]->UpdateAux(q, ViewerLocation, Error[5]);
		}
	}
	
	// Test for disabling.  East, South, and center.
	if ((EnabledFlags & 1) && SubEnabledCount[0] == 0 && VertexTest(cd.xorg + whole, Vertex[1].Y, cd.zorg + half, Error[0], ViewerLocation) == false) {
		EnabledFlags &= ~1;
		quadsquare*	s = GetNeighbor(0, cd);
		if (s) s->EnabledFlags &= ~4;
	}
	if ((EnabledFlags & 8) && SubEnabledCount[1] == 0 && VertexTest(cd.xorg + half, Vertex[4].Y, cd.zorg + whole, Error[1], ViewerLocation) == false) {
		EnabledFlags &= ~8;
		quadsquare*	s = GetNeighbor(3, cd);
		if (s) s->EnabledFlags &= ~2;
	}
	if (EnabledFlags == 0 &&
	    cd.Parent != NULL &&
	    BoxTest(cd.xorg, cd.zorg, whole, MinY, MaxY, CenterError, ViewerLocation) == false)
	{
		// Disable ourself.
		cd.Parent->Square->NotifyChildDisable(*cd.Parent, cd.ChildIndex);	// nb: possibly deletes 'this'.
	}
}


float	VertexArray[9 * 3];
unsigned int	ColorArray[9];
unsigned char	VertList[24];
int	TriCount = 0;
	


static void	InitVert(int index, float x, float y, float z)
// Initializes the indexed vertex of VertexArray[] with the
// given values.
{
	int	i = index * 3;
	VertexArray[i] = x;
	VertexArray[i+1] = y;
	VertexArray[i+2] = z;
}


int	quadsquare::Render(const quadcornerdata& cd)
// Draws the heightfield represented by this tree.
// Returns the number of triangles rendered.
{
	TriCount = 0;

	// Do some initial setup, then do all the work in RenderAux().
//	gl....();
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, VertexArray);

	//xxxx
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScalef(1.0, VERTICAL_SCALE, 1.0);
	
	// No texture; use crummy vertex lighting.
	glEnableClientState(GL_NORMAL_ARRAY);
	glColorPointer(3, GL_UNSIGNED_BYTE, 0, ColorArray);
       
		// Set up automatic texture-coordinate generation.
		// Basically we're just stretching the current texture over the entire 64K x 64K terrain.
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		float	p[4] = { 1.0 / 65536, 0, 0, 0 };
		glTexGenfv(GL_S, GL_OBJECT_PLANE, p);

		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		p[0] = 0;	p[2] = 1.0 / 65536;
		glTexGenfv(GL_T, GL_OBJECT_PLANE, p);

		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);


	RenderAux(cd, GFX_PARTIALLY_VISIBLE);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
//	gl...();

	glPopMatrix();

	return TriCount;
}


void	quadsquare::RenderAux(const quadcornerdata& cd,  CLIPSTATE vis)
// Does the work of rendering this square.  Uses the enabled vertices only.
// Recurses as necessary.
{
	int	half = 1 << cd.Level;
	int	whole = 2 << cd.Level;
	
	// If this square is outside the frustum, then don't render it.
	if (vis != GFX_TOTALLY_VISIBLE) {
		Vector	min, max;
		min.i = cd.xorg;
		min.j = MinY * VERTICAL_SCALE;
		min.k = cd.zorg;
		max.i = cd.xorg + whole;
		max.j = MaxY * VERTICAL_SCALE;
		max.k = cd.zorg + whole;
		vis = GFXBoxInFrustum(min, max);
		if (vis == GFX_NOT_VISIBLE) {
			// This square is completely outside the view frustum.
			return;
		}
		// else vis is either NO_CLIP or SOME_CLIP.  If it's NO_CLIP, then child
		// squares won't have to bother with the frustum check.
	}
	
	int	i;

	int	flags = 0;
	int	mask = 1;
	quadcornerdata	q;
	for (i = 0; i < 4; i++, mask <<= 1) {
		if (EnabledFlags & (16 << i)) {
			SetupCornerData(&q, cd, i);
			Child[i]->RenderAux(q, vis);
		} else {
			flags |= mask;
		}
	}

	if (flags == 0) return;

	// Init vertex data.
	InitVert(0, cd.xorg + half, Vertex[0].Y, cd.zorg + half);
	InitVert(1, cd.xorg + whole, Vertex[1].Y, cd.zorg + half);
	InitVert(2, cd.xorg + whole, cd.Verts[0].Y, cd.zorg);
	InitVert(3, cd.xorg + half, Vertex[2].Y, cd.zorg);
	InitVert(4, cd.xorg, cd.Verts[1].Y, cd.zorg);
	InitVert(5, cd.xorg, Vertex[3].Y, cd.zorg + half);
	InitVert(6, cd.xorg, cd.Verts[2].Y, cd.zorg + whole);
	InitVert(7, cd.xorg + half, Vertex[4].Y, cd.zorg + whole);
	InitVert(8, cd.xorg + whole, cd.Verts[3].Y, cd.zorg + whole);

	int	vcount = 0;
	
// Local macro to make the triangle logic shorter & hopefully clearer.
#define tri(a,b,c) ( VertList[vcount++] = a, VertList[vcount++] = b, VertList[vcount++] = c )

	// Make the list of triangles to draw.
	if ((EnabledFlags & 1) == 0) tri(0, 8, 2);
	else {
		if (flags & 8) tri(0, 8, 1);
		if (flags & 1) tri(0, 1, 2);
	}
	if ((EnabledFlags & 2) == 0) tri(0, 2, 4);
	else {
		if (flags & 1) tri(0, 2, 3);
		if (flags & 2) tri(0, 3, 4);
	}
	if ((EnabledFlags & 4) == 0) tri(0, 4, 6);
	else {
		if (flags & 2) tri(0, 4, 5);
		if (flags & 4) tri(0, 5, 6);
	}
	if ((EnabledFlags & 8) == 0) tri(0, 6, 8);
	else {
		if (flags & 4) tri(0, 6, 7);
		if (flags & 8) tri(0, 7, 8);
	}

	// Draw 'em.
	glDrawElements(GL_TRIANGLES, vcount, GL_UNSIGNED_BYTE, VertList);

	// Count 'em.
	TriCount += vcount / 3;
}


void	quadsquare::SetupCornerData(quadcornerdata* q, const quadcornerdata& cd, int ChildIndex)
// Fills the given structure with the appropriate corner values for the
// specified child block, given our own vertex data and our corner
// vertex data from cd.
//
// ChildIndex mapping:
// +-+-+
// |1|0|
// +-+-+
// |2|3|
// +-+-+
//
// Verts mapping:
// 1-0
// | |
// 2-3
//
// Vertex mapping:
// +-2-+
// | | |
// 3-0-1
// | | |
// +-4-+
{
	int	half = 1 << cd.Level;

	q->Parent = &cd;
	q->Square = Child[ChildIndex];
	q->Level = cd.Level - 1;
	q->ChildIndex = ChildIndex;
	
	switch (ChildIndex) {
	default:
	case 0:
		q->xorg = cd.xorg + half;
		q->zorg = cd.zorg;
		q->Verts[0] = cd.Verts[0];
		q->Verts[1] = Vertex[2];
		q->Verts[2] = Vertex[0];
		q->Verts[3] = Vertex[1];
		break;

	case 1:
		q->xorg = cd.xorg;
		q->zorg = cd.zorg;
		q->Verts[0] = Vertex[2];
		q->Verts[1] = cd.Verts[1];
		q->Verts[2] = Vertex[3];
		q->Verts[3] = Vertex[0];
		break;

	case 2:
		q->xorg = cd.xorg;
		q->zorg = cd.zorg + half;
		q->Verts[0] = Vertex[0];
		q->Verts[1] = Vertex[3];
		q->Verts[2] = cd.Verts[2];
		q->Verts[3] = Vertex[4];
		break;

	case 3:
		q->xorg = cd.xorg + half;
		q->zorg = cd.zorg + half;
		q->Verts[0] = Vertex[1];
		q->Verts[1] = Vertex[0];
		q->Verts[2] = Vertex[4];
		q->Verts[3] = cd.Verts[3];
		break;
	}	
}


void	quadsquare::AddHeightMap(const quadcornerdata& cd, const HeightMapInfo& hm)
// Sets the height of all samples within the specified rectangular
// region using the given array of floats.  Extends the tree to the
// level of detail defined by (1 << hm.Scale) as necessary.
{
	// If block is outside rectangle, then don't bother.
	int	BlockSize = 2 << cd.Level;
	if (cd.xorg > hm.XOrigin + ((hm.XSize + 2) << hm.Scale) ||
	    cd.xorg + BlockSize < hm.XOrigin - (1 << hm.Scale) ||
	    cd.zorg > hm.ZOrigin + ((hm.ZSize + 2) << hm.Scale) ||
	    cd.zorg + BlockSize < hm.ZOrigin - (1 << hm.Scale))
	{
		// This square does not touch the given height array area; no need to modify this square or descendants.
		return;
	}

	if (cd.Parent && cd.Parent->Square) {
		cd.Parent->Square->EnableChild(cd.ChildIndex, *cd.Parent);	// causes parent edge verts to be enabled, possibly causing neighbor blocks to be created.
	}
	
	int	i;
	
	int	half = 1 << cd.Level;

	// Create and update child nodes.
	for (i = 0; i < 4; i++) {
		quadcornerdata	q;
		SetupCornerData(&q, cd, i);
				
		if (Child[i] == NULL && cd.Level > hm.Scale) {
			// Create child node w/ current (unmodified) values for corner verts.
			Child[i] = new quadsquare(&q);
		}
		
		// Recurse.
		if (Child[i]) {
			Child[i]->AddHeightMap(q, hm);
		}
	}
	
	// Deviate vertex heights based on data sampled from heightmap.
	float	s[5];
	s[0] = hm.Sample(cd.xorg + half, cd.zorg + half);
	s[1] = hm.Sample(cd.xorg + half*2, cd.zorg + half);
	s[2] = hm.Sample(cd.xorg + half, cd.zorg);
	s[3] = hm.Sample(cd.xorg, cd.zorg + half);
	s[4] = hm.Sample(cd.xorg + half, cd.zorg + half*2);

	// Modify the vertex heights if necessary, and set the dirty
	// flag if any modifications occur, so that we know we need to
	// recompute error data later.
	for (i = 0; i < 5; i++) {
		if (s[i] != 0) {
			Dirty = true;
			Vertex[i].Y += s[i];
		}
	}

	if (!Dirty) {
		// Check to see if any child nodes are dirty, and set the dirty flag if so.
		for (i = 0; i < 4; i++) {
			if (Child[i] && Child[i]->Dirty) {
				Dirty = true;
				break;
			}
		}
	}

	if (Dirty) SetStatic(cd);
}


//
// HeightMapInfo
//


float	HeightMapInfo::Sample(int x, int z) const
// Returns the height (y-value) of a point in this heightmap.  The given (x,z) are in
// world coordinates.  Heights outside this heightmap are considered to be 0.  Heights
// between sample points are bilinearly interpolated from surrounding points.
// xxx deal with edges: either force to 0 or over-size the query region....
{
	// Break coordinates into grid-relative coords (ix,iz) and remainder (rx,rz).
	
	int	ix = (x - XOrigin) >> Scale;
	int	iz = (z - ZOrigin) >> Scale;

	int	mask = (1 << Scale) - 1;

	int	rx = (x - XOrigin) & mask;
	int	rz = (z - ZOrigin) & mask;

	if (ix < 0 || ix >= XSize-1 || iz < 0 || iz >= ZSize-1) return 0;	// Outside the grid.

	float	fx = float(rx) / (mask + 1);
	float	fz = float(rz) / (mask + 1);

	float	s00 = Data[ix + iz * RowWidth];
	float	s01 = Data[(ix+1) + iz * RowWidth];
	float	s10 = Data[ix + (iz+1) * RowWidth];
	float	s11 = Data[(ix+1) + (iz+1) * RowWidth];

	return (s00 * (1-fx) + s01 * fx) * (1-fz) +
		(s10 * (1-fx) + s11 * fx) * fz;
}

