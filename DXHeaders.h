#define DX_FILE_VERSION 105
#define MAX_NAME 30
#define MAX_TEXTURE_NAME 100

#define INVALID_VALUE ((UINT16)-1)


#define ROOT_FRAME_NAME "SCENE_ROOT"
#define SETNAME(destination, source) memcpy_s(destination, MAX_NAME, source, MAX_NAME)


#include "vector"


struct P4
{
	float x, y, z, w;
};


struct P3
{
	float x, y, z;
};


struct P2
{
	float x, y;
};





struct I4
{
	UINT16 x, y, z, w;
};


struct I3
{
	UINT16 x, y, z;
};


struct I2
{
	UINT16 x, y;
};





struct DX_VERTEX
{
	P3 Position;
	P3 Normal;
	P2 Tex;
};


struct DX_VERTEX_SKIN
{
	P3 Position;
	P3 Normal;
	P2 Tex;
	P4 BlendWeights;
	I4 BlendIndices;
};




/* FILE STRUCTURE
	DX_HEADER						File Header
--------------------- Non Buffer Data --------------------------------
	DX_FRAME						Frames in file
	DX_MESH							Meshes in file
	DX_SUBSET						Subsets in file
	DX_MATEIRALS					Materials in file
	DX_VERTEX_BUFFER_HEADER			Vertex buffers in file
	DX_INDEX_BUFFER_HEADER			Index buffers in file
-----------------------------------------------------------------------

---------------------- Buffer Data ------------------------------------
	BYTE							Subset data
	DX_VERTEX/DX_VERTEX_SKIN		Vertices data
	UINT16							Indices data
-----------------------------------------------------------------------

----------------------- Animation Data --------------------------------
	DX_ANIMATION_HEADER				// Animation header
	DX_ANIMATION_FRAME				// Frames in animation
	DX_ANIMATION_KEY				// Keys data
*/





//---------------------------- NOTES: -------------------------------------
//	Name of Subset and its corresponding Material is same
//	# of Meshes and # of VBs and IBs are same


// the rotation animation keys need to be checked & adjusted
// before they can be used for animation. the check & adjust
// algorithm is as follows:

// The problem is that 3dsmax outputs rotation quaternion by
// ranging the angle b/w -180.0 & 180.0. While we need it b/w
// 0.0 & 360.

// Algorithm checks the dot product of the current & previous
// rotation keys. If it is less than 0.0. Then it means that
// the animaton controller will lerp b/w the keys using the longer
// route. For example. An object rotates anti-clock wise 360. It
// has 5 keys. One at 0 degree, one at 90, at 180, at 270 and at 360.
// 3dsmax outputs 270 and 360 degree as -90 and 0.
//	So here the object will first rotate 180 degrees anti-clock wise
// & then move clockwise to -90 & then back to 0 anti-clock wise
//
// This is the longer route. To go through the shorter route. i.e.
// from 180 to 270 degrees ( a 90 degree rotation ), we need to
// adjust the quaternion. To do this we first take the inverse of the
// current key & then multiply its angle by -1.0f. This gives the
// same result as subtracting the angles of the rotation from 360
//---------------------------------------------------------------
//	LOOP THROUGH THE KEYS
//
//		IF THIS IS NOT THE FIRST KEY
//			CUR		=	CURRENT KEY QUATERNION
//			PREV	=	PREVIOUS KEY QUATERNION
//			DOT		=	DOT PRODUCT OF  CUR and PREV
//		
//			IF DOT < 0.0
//				ADJUSTED	=	INVERSE OF CUR
//				ADJUSTED.W	=	ADJUSTED.W * -1.0
//				CUR			=	ADJUSTED
//
//			END IF
//
//		ENDIF
//
//	END LOOP
//-----------------------------------------------------------------




struct DX_HEADER
{
	UINT32 Version;					// file version
	UINT64 HeaderSize;				// size of this struct
	UINT64 NonBufferDataSize;		// size of Frame Mesh Subset Material Vertex & Index data
	UINT64 BufferDataSize;			// size of vertices and indices

	UINT16 NumVertexBuffers;		// no of vertex buffers
	UINT16 NumIndexBuffers;			// no of index buffers
	UINT16 NumMeshes;				// no of meshes
	UINT16 NumTotalSubsets;			// no of subsets
	UINT16 NumFrames;				// no of frames
	UINT16 NumMaterials;			// no of materials

	UINT64 FrameDataOffset;			// offset of frame data from START
	UINT64 MeshDataOffset;			// offset of mesh data from START
	UINT64 SubsetDataOffset;		// offset of subset data from START
	UINT64 MaterialDataOffset;		// offset of material data from START
	UINT64 VertexDataOffset;		// offset of vertex header data from START
	UINT64 IndexDataOffset;			// offset of index header data from START
	UINT64 BufferDataOffset;		// offset of buffer data from START
};








struct DX_VERTEX_BUFFER_HEADER
{
	bool Skinned;
	UINT16 NumVertices;		// no of vertices in buffer
	UINT64 SizeBytes;		// total size of buffer
	UINT16 StrideBytes;		// size of one vertex in buffer
	UINT64 DataOffset;		// offset of data from the BufferDataOffset
};





struct DX_INDEX_BUFFER_HEADER
{
	UINT16 NumIndices;		// no of indices in buffer
	UINT64 SizeBytes;		// sizeof index data (16bit indices are used)
	UINT64 DataOffset;		// offset of data from the BufferDataOffset
};




struct DX_MESH
{
	char Name[MAX_NAME];	// name of the mesh
	UINT16 VertexBuffer;	// id of vertex buffer used
	UINT16 IndexBuffer;		// id of index buffer used
	UINT16 NumSubsets;		// no of subsets used
	UINT64 SubsetOffset;	// subset data offset from BufferDataOffset (16bit subset data used)
	UINT16 NumBones;		// number of bones contained by this mesh
	UINT16 FirstBoneFrame;	// the id of the first bone frame used by this mesh
};






struct DX_SUBSET
{
	char Name[MAX_NAME];	// name of subset
	UINT16 MaterialID;		// material id used
	UINT16 IndexStart;		// start of indices used
	UINT16 IndexCount;		// indices used
};





struct DX_FRAME
{
	char Name[MAX_NAME];	// name
	UINT16 Mesh;			// Id of mesh used
	UINT16 ParentFrame;		// parent frame
	UINT16 ChildFrame;		// children frame
	UINT16 SiblingFrame;	// sibling frame
	D3DXMATRIX Matrix;		// world transform matrix
};



struct DX_BONE
{
	UINT16 Id;
	UINT16 ParentId;
	UINT16 ChildId;
	UINT16 SiblingId;
	char Name[MAX_NAME];
	char Parent[MAX_NAME];
	char Child[MAX_NAME];
	char Sibling[MAX_NAME];
	D3DXMATRIX XForm;
};




struct DX_MATERIAL
{
	char Name[MAX_NAME];	// name of material
	P4 Diffuse;				// diffuse data
	P4 Ambient;				// ambient data
	P4 Specular;			// specular data
	P4 Emissive;			// emissive data
	float Power;			// power

	// diffuse map
	char Texture1[MAX_TEXTURE_NAME];
	// bump map
	char Texture2[MAX_TEXTURE_NAME];
	// opacity
	char Texture3[MAX_TEXTURE_NAME];

	UINT64 uTex1;
	UINT64 uTex2;
	UINT64 uTex3;
};







struct DX_ANIMATION_HEADER
{
	UINT32 Version;
	char Name[MAX_NAME];
	UINT16 NumFrames;
	UINT32 NumAnimationKeys;
	UINT16 AnimationFPS;
	UINT64 FrameDataOffset;
	UINT64 FrameDataSize;
	UINT64 AnimationDataSize;
	UINT64 AnimationDataOffset;
};



struct DX_ANIMATION_FRAME
{
	UINT16 Frame;
	UINT32 NumKeys;
	UINT32 FirstKey;
};



struct KEY
{
	P3 Position;
	P4 Rotation;
	P3 Scale;
};



struct DX_ANIMATION_KEY
{
	UINT32 Frame;
	KEY Data;
};



struct DX_ANIMATION_DATA
{
	DX_ANIMATION_HEADER header;
	std::vector<DX_ANIMATION_FRAME> frames;
	std::vector<DX_ANIMATION_KEY> keys;
};


UINT StructMeshSize = sizeof(DX_MESH);
UINT StructSubsetSize = sizeof(DX_SUBSET);
UINT StructFrameSize = sizeof(DX_FRAME);
UINT StructHeaderSize = sizeof(DX_HEADER);
UINT StructMaterialSize = sizeof(DX_MATERIAL);
UINT StructVertexSize = sizeof(DX_VERTEX);
UINT StructVBSize = sizeof(DX_VERTEX_BUFFER_HEADER);
UINT StructIBSize = sizeof(DX_INDEX_BUFFER_HEADER);

UINT StructAnimationHeaderSize = sizeof(DX_ANIMATION_HEADER);
UINT StructAnimationFrameSize = sizeof(DX_ANIMATION_FRAME);
UINT StructKeySize = sizeof(DX_ANIMATION_KEY);