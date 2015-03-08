// Title: DXMesh exporter for 3ds max 2010
// Date Modified: 1 Jan 10



//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Appwizard generated plugin
//				3DXI file exporter project template
//				For a more in-depth exemple of a 3DXI exporter,
//				please refer to maxsdk\samples\igame\export.
// AUTHOR:		Jean-Francois Yelle - created Mar.20.2007
//***************************************************************************/

#include "DXExporter.h"

#include "3dsmaxsdk_preinclude.h"
#include "IGame/IGame.h"
#include "IGame/IGameModifier.h"
#include <d3d9.h>
#include <d3dx9.h>
#include "DXHeaders.h"
#include "vector"


#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")


// struct to store mesh header, vertices, indices, and subset ids of a mesh
struct CMesh
{
	BOOL bCollision;
	DX_MESH m_Mesh;
	std::vector <DX_VERTEX> m_Vertices;
	std::vector <DX_VERTEX_SKIN> m_SkinnedVertices;
	std::vector <P3> m_CollisionPoints;
	std::vector <UINT16> m_Indices;
	std::vector <UINT16> m_Subsets;
	std::vector <DX_BONE> m_Bones;
};



// struct to store Animation detail
struct AnimInfo
{
	char Name[MAX_NAME];
	UINT Start;
	UINT End;
	FLOAT FPS;
	UINT Step;
};


// vector of animation details
std::vector<AnimInfo> Animations;


bool bExportAnimation;
bool Export;



#define DXExporter_CLASS_ID	Class_ID(0x7a8da11e, 0x996bd86e)






// Dummy function for progress bar
DWORD WINAPI fn(LPVOID arg)
{
	return(0);
}






class DXExporter : public SceneExport 
{
	public:
		
		virtual int				ExtCount();					// Number of extensions supported
		virtual const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
		virtual const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
		virtual const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
		virtual const TCHAR *	AuthorName();				// ASCII Author name
		virtual const TCHAR *	CopyrightMessage();			// ASCII Copyright message
		virtual const TCHAR *	OtherMessage1();			// Other message #1
		virtual const TCHAR *	OtherMessage2();			// Other message #2
		virtual unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
		virtual void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box

		virtual BOOL SupportsOptions(int ext, DWORD options);
		virtual int	DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);

		//Constructor/Destructor
		DXExporter();
		virtual ~DXExporter();

		float m_Scale;


	protected:
		int GetSceneNodes(INodeTab& i_nodeTab, INode* i_currentNode =NULL);

	private:
		IGameScene * pIgame;

		FILE* m_File;
		DX_HEADER FileHeader;
		std::vector <DX_FRAME> m_Frames;
		std::vector <CMesh> m_Meshes;
		std::vector <DX_MATERIAL> m_Materials;
		std::vector <DX_SUBSET> m_Subsets;
		std::vector <DX_VERTEX_BUFFER_HEADER> m_VertexBuffers;
		std::vector <DX_INDEX_BUFFER_HEADER> m_IndexBuffers;

		std::vector < std::vector<DX_BONE>* > m_BonesList;

		// vector to store actual animations data
		std::vector <DX_ANIMATION_DATA> m_AnimationData;


		// returns the # of children of the frame
		UINT16 GetNumChildren(UINT16 ParentFrame);
		// gets the children of the frame
		void GetChildren(UINT16 ParentFrame, UINT16* childrenIDs);


		// returns the id of the specified frame
		UINT16 FindFrame(char* name, UINT16 parentFrame);
		// returns the id of the specified frame
		UINT16 FindFrame(char* name);
		// returns the id of the specified material
		UINT16 FindMaterial(char* name);
		// returns the id of the specified subset
		UINT16 FindSubset(char* name);
		


		// adds a frame to the export list
		UINT16 AddFrame(DX_FRAME* frame);
		// adds a material to the export list
		UINT16 AddMaterial(DX_MATERIAL* mat);
		// adds a subset to the export list
		UINT16 AddSubset(DX_SUBSET* subset);
		// adds a vb to the export list
		UINT16 AddVertexBuffer(DX_VERTEX_BUFFER_HEADER* vb);
		// adds an ib to the export list
		UINT16 AddIndexBuffer(DX_INDEX_BUFFER_HEADER* ib);
		// adds a single bone from the skinned mesh to the bones list
		UINT16 AddSingleBoneFromSkinnedMeshToBoneList(IGameSkin* pSkinnedMesh, std::vector <DX_BONE>* Bones, CMesh* pMesh, UINT boneIndex);


		
		
		DX_VERTEX_SKIN SetupSkinVertex(CMesh* pMesh, IGameSkin* pSkinnedMesh, UINT16 vertexIndex);
		// sets up a mesh and its vertex and index buffers and bones
		UINT16 SetupMesh(IGameNode* pNode, const GMatrix* mWorld, BOOL bCollisionMesh);
		// Setup the frame's child's sibling values
		void SetFrameChildSiblings(UINT16 pFrame);
		// sets up the specified animation for a frame
		void SetupAnimation(IGameNode* pNode, IGameControl* pNodeControl, AnimInfo* pAnimInfo, UINT AnimDataId);
		// Setup a frame and its child frames
		UINT16 SetupFrame(IGameNode* pNode, UINT16 ParentFrame, bool AnimationOnly, AnimInfo* pAnimInfo, UINT AnimDataId);
		// sets up bones frames and bones in a skinned mesh
		void SetupBoneFrames(CMesh* pMesh, IGameSkin* pSkinnedMesh);
		// sets up a single DX_BONE struct
		DX_BONE SetupBoneStruct(IGameNode* pBoneNode);
				
		static HWND hParams;
};



class DXExporterClassDesc : public ClassDesc2 
{
public:
	virtual int IsPublic() 							{ return TRUE; }
	virtual void* Create(BOOL /*loading = FALSE*/) 		{ return new DXExporter(); }
	virtual const TCHAR *	ClassName() 			{ return GetString(IDS_CLASS_NAME); }
	virtual SClass_ID SuperClassID() 				{ return SCENE_EXPORT_CLASS_ID; }
	virtual Class_ID ClassID() 						{ return DXExporter_CLASS_ID; }
	virtual const TCHAR* Category() 				{ return GetString(IDS_CATEGORY); }

	virtual const TCHAR* InternalName() 			{ return _T("DXExporter"); }	// returns fixed parsable name (scripter-visible name)
	virtual HINSTANCE HInstance() 					{ return hInstance; }					// returns owning module handle
	

};


ClassDesc2* GetDXExporterDesc() { 
	static DXExporterClassDesc DXExporterDesc;
	return &DXExporterDesc; 
}





INT_PTR CALLBACK DXExporterOptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static DXExporter *imp = NULL;
	ISpinnerControl* spin;

	LRESULT lr;
	int id = (int)-1;

	std::vector<AnimInfo>::iterator it;

	switch(message) {
		case WM_INITDIALOG:
			Export = false;

			imp = (DXExporter *)lParam;
			CenterWindow(hWnd,GetParent(hWnd));
			spin = GetISpinner( GetDlgItem( hWnd, IDC_SCALE_SPIN ) );
			spin->LinkToEdit( GetDlgItem( hWnd, IDC_SCALE_EDIT ), EDITTYPE_FLOAT );
			spin->SetLimits(0.0f, 1000.0f, 1);
			spin->SetScale(1.0f);
			spin->SetValue(39.370f, 0);
			ReleaseISpinner(spin);

			spin = GetISpinner( GetDlgItem( hWnd, IDC_A_START_SPIN ) );
			spin->LinkToEdit( GetDlgItem( hWnd, IDC_A_START_EDIT ), EDITTYPE_INT );
			spin->SetLimits(0, 100000);
			spin->SetValue(0, 0);
			ReleaseISpinner(spin);

			spin = GetISpinner( GetDlgItem( hWnd, IDC_A_END_SPIN ) );
			spin->LinkToEdit( GetDlgItem( hWnd, IDC_A_END_EDIT ), EDITTYPE_INT );
			spin->SetLimits(0, 100000);
			spin->SetValue(60, 0);
			ReleaseISpinner(spin);

			spin = GetISpinner( GetDlgItem( hWnd, IDC_A_FPS_SPIN ) );
			spin->LinkToEdit( GetDlgItem( hWnd, IDC_A_FPS_EDIT ), EDITTYPE_FLOAT );
			spin->SetLimits(0.0f, 600.0f);
			spin->SetScale(1.0f);
			spin->SetValue(30.0f, 0);
			ReleaseISpinner(spin);

			spin = GetISpinner( GetDlgItem( hWnd, IDC_A_STEP_SPIN ) );
			spin->LinkToEdit( GetDlgItem( hWnd, IDC_A_STEP_EDIT ), EDITTYPE_INT );
			spin->SetLimits(0, 100);
			spin->SetValue(3, 0);
			ReleaseISpinner(spin);

			SendMessage(GetDlgItem(hWnd, IDC_A_NAME_EDIT), EM_SETLIMITTEXT, 30, 0);
			bExportAnimation = false;
			
			return TRUE;

		case WM_COMMAND:
			switch ( LOWORD( wParam ) )
			{
			case IDC_EXPORT:
				Export = true;
				spin = GetISpinner( GetDlgItem( hWnd, IDC_SCALE_SPIN ) );
				imp->m_Scale = spin->GetFVal();
				ReleaseISpinner(spin);
				EndDialog(hWnd, 0);
				return 1;


			case IDC_BTN_ADDANIM:
				AnimInfo info;
				info.Start = GetISpinner(GetDlgItem(hWnd, IDC_A_START_SPIN))->GetIVal();
				info.End = GetISpinner(GetDlgItem(hWnd, IDC_A_END_SPIN))->GetIVal();
				info.FPS = GetISpinner(GetDlgItem(hWnd, IDC_A_FPS_SPIN))->GetFVal();
				info.Step = GetISpinner(GetDlgItem(hWnd, IDC_A_STEP_SPIN))->GetIVal();
				if (info.Start > info.End)
				{
					info.Start = info.End;
					break;
				}
				GetDlgItemText(hWnd, IDC_A_NAME_EDIT, info.Name, MAX_NAME);
				Animations.push_back(info);
				char string[100];
				sprintf(string, "%u %s Start:%u End:%u FPS:%f  Sample Rate:%u", Animations.size(), info.Name, info.Start, info.End, info.FPS, info.Step);
				SendMessage(GetDlgItem(hWnd, IDC_LIST_ANIMS), LB_ADDSTRING, 0, (LPARAM)string);
				break;


			case IDC_BTN_DELANIM:
				id = SendMessage(GetDlgItem(hWnd, IDC_LIST_ANIMS), LB_GETCURSEL, 0, 0);
				if (id != ((int)-1))
				{
					SendMessage(GetDlgItem(hWnd, IDC_LIST_ANIMS), LB_DELETESTRING, (WPARAM)id, 0);
					it = Animations.begin() + id;
					Animations.erase(it);
				}
				break;


			case IDC_CHK_EXPORTANIM:
				lr = SendMessage( GetDlgItem(hWnd, IDC_CHK_EXPORTANIM), BM_GETCHECK, 0, 0);
				if (lr == BST_CHECKED)
				{
					bExportAnimation = true;
				}
				else
				{
					bExportAnimation = false;
				}
				break;
			}
			break;

		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return 0;
	}
	return 0;
}


//--- DXExporter -------------------------------------------------------
DXExporter::DXExporter()
{
	// create a SceenRoot frame which will be at the top of
	// the frames hirerchey, (Frame # 0)
	DX_FRAME SceneRoot;
	D3DXMatrixIdentity(&SceneRoot.Matrix);
	SETNAME(SceneRoot.Name, ROOT_FRAME_NAME);
	SceneRoot.Mesh = INVALID_VALUE;
	D3DXMatrixIdentity(&SceneRoot.Matrix);
	SceneRoot.ParentFrame = INVALID_VALUE;
	SceneRoot.SiblingFrame = INVALID_VALUE;
	SceneRoot.ChildFrame = 1;
	AddFrame(&SceneRoot);

	// make a default material for those subsets that do not
	// have a material applied to them
	DX_MATERIAL DefaultMat;
	SETNAME(DefaultMat.Name, "Default");
	P4 def0 = { 0.5f, 0.5f, 0.5f, 1.0f };
	P4 def1 = { 0.0f, 0.0f, 0.0f, 1.0f };
	DefaultMat.Diffuse = def0;
	DefaultMat.Ambient = def0;
	DefaultMat.Emissive = def1;
	DefaultMat.Specular = def1;
	DefaultMat.Power = 0.0f;
	DefaultMat.Texture1[0] = '\0';
	DefaultMat.Texture2[0] = '\0';
	DefaultMat.Texture3[0] = '\0';
	m_Materials.push_back(DefaultMat);

	FileHeader.HeaderSize = sizeof(DX_HEADER);
	FileHeader.Version = DX_FILE_VERSION;

	Animations.clear();
}

DXExporter::~DXExporter() 
{

}

int DXExporter::ExtCount()
{
	// Returns the number of file name extensions supported by the plug-in.
	return 1;
}

const TCHAR *DXExporter::Ext(int n)
{		
	return _T("dxmesh");
}

const TCHAR *DXExporter::LongDesc()
{
	return _T("Exports objects in dxmesh format");
}
	
const TCHAR *DXExporter::ShortDesc() 
{			
	return _T("DX mesh file");
}

const TCHAR *DXExporter::AuthorName()
{			
	return _T("Abdullah Baig");
}

const TCHAR *DXExporter::CopyrightMessage() 
{	
	return _T("Copyright (c) 2010");
}

const TCHAR *DXExporter::OtherMessage1() 
{		
	return _T("");
}

const TCHAR *DXExporter::OtherMessage2() 
{		
	return _T("");
}

unsigned int DXExporter::Version()
{				
	return 105;
}

void DXExporter::ShowAbout(HWND hWnd)
{			
	// Optional
}

BOOL DXExporter::SupportsOptions(int ext, DWORD options)
{
	// TODO Decide which options to support.  Simply return
	// true for each option supported by each Extension 
	// the exporter supports.

	return TRUE;
}

int DXExporter::GetSceneNodes(INodeTab& i_nodeTab, INode* i_currentNode /*=NULL*/)
{
	int i;
	if (i_currentNode == NULL)
	{
		i_currentNode = GetCOREInterface()->GetRootNode();
	}
	else // IGame will crash 3ds Max if it is initialised with the root node.
	{
	    i_nodeTab.AppendNode(i_currentNode);
	}
	for (i = 0; i < i_currentNode->NumberOfChildren(); i++)
	{
		GetSceneNodes(i_nodeTab, i_currentNode->GetChildNode(i));
	}
	return i_nodeTab.Count();
}









// Coverts 3dsmax matrix to directX matrix
void ConvertGToDMatrix(GMatrix* gMat, D3DXMATRIX* dMat)
{
	Point4 r0 = gMat->GetRow(0);
	Point4 r1 = gMat->GetRow(1);
	Point4 r2 = gMat->GetRow(2);
	Point4 r3 = gMat->GetRow(3);

	dMat->_11 = r0.x;
	dMat->_12 = r0.y;
	dMat->_13 = r0.z;
	dMat->_14 = r0.w;

	dMat->_21 = r1.x;
	dMat->_22 = r1.y;
	dMat->_23 = r1.z;
	dMat->_24 = r1.w;

	dMat->_31 = r2.x;
	dMat->_32 = r2.y;
	dMat->_33 = r2.z;
	dMat->_34 = r2.w;

	dMat->_41 = r3.x;
	dMat->_42 = r3.y;
	dMat->_43 = r3.z;
	dMat->_44 = r3.w;
}



// converts IGameMaterial to DX_MATERIAL type
DX_MATERIAL ConvertIGameMaterial(IGameMaterial* pMaterial)
{
	DX_MATERIAL dxMat;
	IGameMaterial* mat = pMaterial;
	char* name = mat->GetMaterialName();
	SETNAME(dxMat.Name, name);

	IGameProperty* pDiff = mat->GetDiffuseData();
	IGameProperty* pAmb = mat->GetAmbientData();
	IGameProperty* pSpec = mat->GetSpecularData();
	IGameProperty* pEmiss = mat->GetEmissiveData();
	IGameProperty* pOpac = mat->GetOpacityData();
	IGameProperty* pGloss = mat->GetGlossinessData();

	Point3 diff, amb, spec, emiss;
	float opac, power;

	pDiff->GetPropertyValue(diff);
	pAmb->GetPropertyValue(amb);
	pSpec->GetPropertyValue(spec);
	pEmiss->GetPropertyValue(emiss);
	pOpac->GetPropertyValue(opac);
	pGloss->GetPropertyValue(power);

	dxMat.Diffuse.x = diff.x;
	dxMat.Diffuse.y = diff.y;
	dxMat.Diffuse.z = diff.z;
	dxMat.Diffuse.w = opac;

	dxMat.Ambient.x = amb.x;
	dxMat.Ambient.y = amb.y;
	dxMat.Ambient.z = amb.z;
	dxMat.Ambient.w = 1.0f;

	dxMat.Specular.x = spec.x;
	dxMat.Specular.y = spec.y;
	dxMat.Specular.z = spec.z;
	dxMat.Specular.w = 1.0f;

	dxMat.Emissive.x = emiss.x;
	dxMat.Emissive.y = emiss.y;
	dxMat.Emissive.z = emiss.z;
	dxMat.Emissive.w = 1.0f;

	dxMat.Power = power;


	dxMat.Texture1[0] = '\0';
	dxMat.Texture2[0] = '\0';
	dxMat.Texture3[0] = '\0';


	int maps = mat->GetNumberOfTextureMaps();
	for (int i=0; i<maps; i++)
	{
		IGameTextureMap* iGameMap = mat->GetIGameTextureMap(i);
		int slot = iGameMap->GetStdMapSlot();
		if (slot == ID_DI)
		{
			char* fileName = iGameMap->GetBitmapFileName();
			char* lastSlash = strrchr(fileName, '\\');
			strcpy(dxMat.Texture1, lastSlash + 1);
		}
		else if (slot == ID_BU)
		{
			char* fileName = iGameMap->GetBitmapFileName();
			char* lastSlash = strrchr(fileName, '\\');
			strcpy(dxMat.Texture2, lastSlash + 1);
		}
		else if (slot == ID_OP)
		{
			char* fileName = iGameMap->GetBitmapFileName();
			char* lastSlash = strrchr(fileName, '\\');
			strcpy(dxMat.Texture3, lastSlash + 1);
		}
	}

	return dxMat;
};












DX_BONE* FindBone(std::vector<DX_BONE>* list, char* name)
{
	for (UINT16 i=0; i<list->size(); i++)
	{
		if (strcmp( list->at(i).Name, name ) == 0)
		{
			return &list->at(i);
		}
	}

	return NULL;
}



DX_BONE* FindBone(std::vector<DX_BONE>* list, UINT16 boneId)
{
	for (UINT16 i=0; i<list->size(); i++)
	{
		if (list->at(i).Id == boneId)
		{
			return &list->at(i);
		}
	}

	return NULL;
}





UINT16 FindBoneId(std::vector<DX_BONE>* list, char* name)
{
	UINT16 id = INVALID_VALUE;
	for (UINT16 i=0; i<list->size(); i++)
	{
		if (strcmp( list->at(i).Name, name ) == 0)
		{
			id = i;
			break;
		}
	}

	return id;
}





UINT16 FindBoneId(std::vector<DX_BONE>* list, UINT16 boneId)
{
	UINT16 id = INVALID_VALUE;
	for (UINT16 i=0; i<list->size(); i++)
	{
		if (list->at(i).Id == boneId)
		{
			id = i;
			break;
		}
	}

	return id;
}




UINT16 AddBoneToList(std::vector<DX_BONE>* list, DX_BONE* bone)
{
	UINT16 id = FindBoneId(list, bone->Name);
	if (id == INVALID_VALUE)
	{
		id = list->size();
		list->push_back(*bone);
	}
	return id;
}




void PutBonesInOrder(std::vector<DX_BONE>*Dest, std::vector<DX_BONE>* Source, UINT16 ParentId)
{
	UINT16 index = FindBoneId(Source, ParentId);
	if (index == INVALID_VALUE)
	{
		return;
	}
	DX_BONE* bone = &Source->at(index);
	AddBoneToList(Dest, bone);
	
	bone = &Source->at(index);
	if (bone->ChildId != INVALID_VALUE)
	{
		PutBonesInOrder(Dest, Source, bone->ChildId);
	}

	bone = &Source->at(index);
	if (bone->SiblingId != INVALID_VALUE)
	{
		PutBonesInOrder(Dest, Source, bone->SiblingId);
	}
}










void AddFrameNumber(std::vector<UINT>*list, UINT frame)
{
	for (UINT i=0; i<list->size(); i++)
	{
		if (list->at(i) == frame)
		{
			return;
		}
	}
	list->push_back(frame);
}























// gets the # of children of a parent
UINT16 DXExporter::GetNumChildren(UINT16 ParentFrame)
{
	UINT16 nChild = 0;

	for (UINT16 i=0; i<m_Frames.size(); i++)
	{
		if (m_Frames.at(i).ParentFrame == ParentFrame)
		{
			nChild++;
		}
	}

	return nChild;
}





// gets the children ids of a frame
void DXExporter::GetChildren(UINT16 ParentFrame, UINT16* childrenIDs)
{
	UINT16 c = 0;
	for (UINT16 i=0; i<m_Frames.size(); i++)
	{
		if (m_Frames.at(i).ParentFrame == ParentFrame)
		{
			childrenIDs[c] = i;
			c++;
		}
	}
}









// finds a frame using its name and parent
UINT16 DXExporter::FindFrame(char* name, UINT16 parentFrame)
{
	for (UINT16 i=0; i<m_Frames.size(); i++)
	{
		if (m_Frames.at(i).ParentFrame == parentFrame)
		{
			char* fName = m_Frames.at(i).Name;
			if ( strcmp(name, fName) == 0 )
			{
				return i;
			}
		}
	}
	return INVALID_VALUE;
}



// finds a frame using its name
UINT16 DXExporter::FindFrame(char* name)
{
	for (UINT16 i=0; i<m_Frames.size(); i++)
	{
		char* fName = m_Frames.at(i).Name;
		if ( strcmp(name, fName) == 0 )
		{
			return i;
		}
	}
	return INVALID_VALUE;
}





UINT16 DXExporter::FindMaterial(char* name)
{
	for (UINT16 m=0; m<(UINT16)m_Materials.size(); m++)
	{
		if ( strcmp(m_Materials.at(m).Name, name) == 0 )
		{
			return m;
		}
	}
	return INVALID_VALUE;
}





UINT16 DXExporter::FindSubset(char* name)
{
	for (UINT16 s=0; s<(UINT16)m_Subsets.size(); s++)
	{
		if ( strcmp(m_Subsets.at(s).Name, name) == 0 )
		{
			return s;
		}
	}
	return INVALID_VALUE;
}











// Adds a vertex to a CMesh structure while checking its existence
UINT16 AddVertex(CMesh* pCMesh, DX_VERTEX* pVertex, DX_VERTEX_SKIN* pSkinVertex, bool bSkinned)
{
	bool bFound = false;
	UINT16 Index = 0;

	if (!bSkinned)
	{
		// loop through the already stored vertices
		for (UINT16 i=0; i<(UINT)pCMesh->m_Vertices.size(); i++)
		{
			// get the vertex at i
			DX_VERTEX* pTVert = &pCMesh->m_Vertices.at(i);
			// if pVertex and this vertex match then a copy already exists
			if (	(pVertex->Position.x == pTVert->Position.x) &&
				(pVertex->Position.y == pTVert->Position.y) &&
				(pVertex->Position.z == pTVert->Position.z) &&
				(pVertex->Normal.x == pTVert->Normal.x) &&
				(pVertex->Normal.y == pTVert->Normal.y) &&
				(pVertex->Normal.z == pTVert->Normal.z) &&
				(pVertex->Tex.x == pTVert->Tex.x) &&
				(pVertex->Tex.y == pTVert->Tex.y)
				)
			{
				bFound = true;
				Index = i;
				break;
			}
		}

		// otherwise add the vertex
		if (!bFound)
		{
			Index = pCMesh->m_Vertices.size();
			pCMesh->m_Vertices.push_back(*pVertex);
		}
	}
	else
	{
		// loop through the already stored vertices
		for (UINT16 i=0; i<(UINT)pCMesh->m_SkinnedVertices.size(); i++)
		{
			// get the vertex at i
			DX_VERTEX_SKIN* pTVert = &pCMesh->m_SkinnedVertices.at(i);
			// if pVertex and this vertex match then a copy already exists
			if (	(pSkinVertex->Position.x == pTVert->Position.x) &&
				(pSkinVertex->Position.y == pTVert->Position.y) &&
				(pSkinVertex->Position.z == pTVert->Position.z) &&
				(pSkinVertex->Normal.x == pTVert->Normal.x) &&
				(pSkinVertex->Normal.y == pTVert->Normal.y) &&
				(pSkinVertex->Normal.z == pTVert->Normal.z) &&
				(pSkinVertex->Tex.x == pTVert->Tex.x) &&
				(pSkinVertex->Tex.y == pTVert->Tex.y)
				)
			{
				bFound = true;
				Index = i;
				break;
			}
		}

		// otherwise add the vertex
		if (!bFound)
		{
			Index = pCMesh->m_SkinnedVertices.size();
			pCMesh->m_SkinnedVertices.push_back(*pSkinVertex);
		}
	}

	return Index;
}





UINT16 DXExporter::AddFrame(DX_FRAME* frame)
{
	UINT16 id = FindFrame(frame->Name);
	if (id == INVALID_VALUE)
	{
		id = m_Frames.size();
		m_Frames.push_back(*frame);
	}
	return id;
}





// adds a material to the materials list
UINT16 DXExporter::AddMaterial(DX_MATERIAL* mat)
{
	UINT16 id = FindMaterial(mat->Name);
	if (id == INVALID_VALUE)
	{
		id = m_Materials.size();
		m_Materials.push_back(*mat);
	}
	return id;
}



// adds a subset to the subsets list
UINT16 DXExporter::AddSubset(DX_SUBSET* subset)
{
	UINT16 id = id = m_Subsets.size();
	m_Subsets.push_back(*subset);
	return id;
}



// adds a vertexbuffer to the vertex buffer list
UINT16 DXExporter::AddVertexBuffer(DX_VERTEX_BUFFER_HEADER* vb)
{
	UINT16 id;
	id = m_VertexBuffers.size();
	m_VertexBuffers.push_back(*vb);
	return id;
}



// adds a indexbuffer to the index buffer list
UINT16 DXExporter::AddIndexBuffer(DX_INDEX_BUFFER_HEADER* ib)
{
	UINT16 id;
	id = m_IndexBuffers.size();
	m_IndexBuffers.push_back(*ib);
	return id;
}




UINT16 DXExporter::AddSingleBoneFromSkinnedMeshToBoneList(IGameSkin* pSkinnedMesh, std::vector <DX_BONE>* Bones, CMesh* pMesh, UINT boneIndex)
{
	DX_BONE curBone;
	
	int numBones = pSkinnedMesh->GetTotalSkinBoneCount();
	if (boneIndex >= numBones)
	{
		return INVALID_VALUE;
	}

	IGameNode* pBone = pSkinnedMesh->GetIGameBone(boneIndex, false);

	curBone = SetupBoneStruct(pBone);

	UINT16 id = AddBoneToList(Bones, &curBone);

	return id;
}





DX_VERTEX_SKIN DXExporter::SetupSkinVertex(CMesh* pMesh, IGameSkin* pSkinnedMesh, UINT16 vertexIndex)
{
	DX_VERTEX_SKIN skinVertex;

	skinVertex.BlendWeights.x = 0.0f;
	skinVertex.BlendWeights.y = 0.0f;
	skinVertex.BlendWeights.z = 0.0f;
	skinVertex.BlendWeights.w = 0.0f;
	skinVertex.BlendIndices.x = INVALID_VALUE;
	skinVertex.BlendIndices.y = INVALID_VALUE;
	skinVertex.BlendIndices.z = INVALID_VALUE;
	skinVertex.BlendIndices.w = INVALID_VALUE;

	UINT16 numBones = pSkinnedMesh->GetNumberOfBones(vertexIndex);

	UINT16 boneIndex[4]; 

	for (UINT16 b=0; b<numBones; b++)
	{
		if (b > 3)
		{
			break;
		}
		UINT16 boneId = pSkinnedMesh->GetBoneID(vertexIndex, b);
		IGameNode* pBoneNode = pIgame->GetIGameNode(boneId);
		char* boneName = pBoneNode->GetName();
		boneIndex[b] = FindBoneId(&pMesh->m_Bones, boneName);
	}


	if (numBones == 1)
	{
		skinVertex.BlendWeights.x = pSkinnedMesh->GetWeight(vertexIndex, 0);
		skinVertex.BlendIndices.x = boneIndex[0];
	}
	else if (numBones == 2)
	{
		skinVertex.BlendWeights.x = pSkinnedMesh->GetWeight(vertexIndex, 0);
		skinVertex.BlendWeights.y = pSkinnedMesh->GetWeight(vertexIndex, 1);
		skinVertex.BlendIndices.x = boneIndex[0];
		skinVertex.BlendIndices.y = boneIndex[1];

	}
	else if (numBones == 3)
	{
		skinVertex.BlendWeights.x = pSkinnedMesh->GetWeight(vertexIndex, 0);
		skinVertex.BlendWeights.y = pSkinnedMesh->GetWeight(vertexIndex, 1);
		skinVertex.BlendWeights.z = pSkinnedMesh->GetWeight(vertexIndex, 2);
		skinVertex.BlendIndices.x = boneIndex[0];
		skinVertex.BlendIndices.y = boneIndex[1];
		skinVertex.BlendIndices.z = boneIndex[2];
	}
	else if (numBones > 3)
	{
		skinVertex.BlendWeights.x = pSkinnedMesh->GetWeight(vertexIndex, 0);
		skinVertex.BlendWeights.y = pSkinnedMesh->GetWeight(vertexIndex, 1);
		skinVertex.BlendWeights.z = pSkinnedMesh->GetWeight(vertexIndex, 2);
		skinVertex.BlendWeights.w = pSkinnedMesh->GetWeight(vertexIndex, 3);
		skinVertex.BlendIndices.x = boneIndex[0];
		skinVertex.BlendIndices.y = boneIndex[1];
		skinVertex.BlendIndices.z = boneIndex[2];
		skinVertex.BlendIndices.w = boneIndex[3];
	}

	return skinVertex;
}






// Gathers meshes's materials, subsets, vertices and indices
UINT16 DXExporter::SetupMesh(IGameNode* pNode, const GMatrix* mWorld, BOOL bCollisionMesh)
{
	bool MultiSet		=	false;
	bool bHasMaterial	=	false;
	bool bIsSkinned		=	false;
	CMesh mMesh;
	IGameMaterial* nodeMaterial	=	NULL;
	IGameObject* pObject		=	NULL;
	IGameMesh* pMesh			=	NULL;
	IGameSkin* pSkinnedMesh		=	NULL;
	UINT16 Subsets = 0;
	
	char* name = pNode->GetName();
	SETNAME(mMesh.m_Mesh.Name, name);
	mMesh.m_Mesh.FirstBoneFrame = INVALID_VALUE;
	mMesh.m_Mesh.NumBones = 0;

	mMesh.bCollision = bCollisionMesh;
	if (!bCollisionMesh)
	{

		int iMat = pNode->GetMaterialIndex();
		if (iMat != -1)
		{
			bHasMaterial = true;
			nodeMaterial = pNode->GetNodeMaterial();
			MultiSet = nodeMaterial->IsSubObjType();
		}
		else
		{
			bHasMaterial = false;
			MultiSet = false;
			nodeMaterial = NULL;
		}
	}

	pObject = pNode->GetIGameObject();
	pMesh = (IGameMesh*)pObject;


	if ( pMesh->InitializeData() )
	{
		if (!bCollisionMesh)
		{
			bIsSkinned = pObject->IsObjectSkinned();

			if (bIsSkinned)
			{
				pSkinnedMesh = pObject->GetIGameSkin();
				SetupBoneFrames(&mMesh, pSkinnedMesh);
			}
			else
			{
				pSkinnedMesh = NULL;
			}

			UINT16 NumVertices = pMesh->GetNumberOfVerts();
			UINT16 NumNormals = pMesh->GetNumberOfNormals();
			UINT16 NumTexCoords = pMesh->GetNumberOfTexVerts();
			UINT16 NumFaces = pMesh->GetNumberOfFaces();

			std::vector<P3> Positions;
			std::vector<P3> Normals;
			std::vector<P2> TexCoords;

			GMatrix gMatWorldInverse;
			gMatWorldInverse = (*mWorld);
			gMatWorldInverse = gMatWorldInverse.Inverse();
			Point3 local;

			// collect vertices
			for (UINT16 v=0; v<NumVertices; v++)
			{
				Point3 point = pMesh->GetVertex(v);
				point.x /= m_Scale;
				point.y /= m_Scale;
				point.z /= m_Scale;
				Point3 transformedPoint = point * gMatWorldInverse;
				P3 p = { transformedPoint.x, transformedPoint.y, transformedPoint.z };
				Positions.push_back(p);
			}

			// collect normals
			for (UINT16 n=0; n<NumNormals; n++)
			{
				Point3 point = pMesh->GetNormal(n);
				P3 p = { point.x, point.y, point.z };
				Normals.push_back(p);
			}

			// collect texcoords
			for (UINT16 t=0; t<NumTexCoords; t++)
			{
				Point2 point = pMesh->GetTexVertex(t);
				P2 p = { point.x, point.y };
				TexCoords.push_back(p);
			}




			if (!MultiSet)
			{
				Subsets = 1;
				DX_SUBSET subset;
				if (bHasMaterial)
				{
					DX_MATERIAL mat;
					mat = ConvertIGameMaterial(nodeMaterial);
					subset.MaterialID = AddMaterial(&mat);
					SETNAME(subset.Name, nodeMaterial->GetMaterialName() );
				}
				else
				{
					subset.MaterialID = 0;
					SETNAME(subset.Name, m_Materials.at(0).Name );
				}
				subset.IndexStart = 0;

				char title[200];
				sprintf_s(title, 200, "MESH:%s,SUBSET:%s", name, subset.Name);
				GetCOREInterface()->ProgressStart(title, TRUE, fn, NULL);

				// collect the indices
				for (UINT16 f=0; f<NumFaces; f++)
				{
					DX_VERTEX vertex;
					DX_VERTEX_SKIN skinVertex;

					FaceEx* face;
					// get the face
					face = pMesh->GetFace(f);

					UINT iPos, iNorm, iTex;
					for(int i=0; i<3; i++)
					{
						iPos = face->vert[i];
						iNorm = face->norm[i];
						iTex = face->texCoord[i];

						vertex.Position = Positions.at(iPos);
						vertex.Normal = Normals.at(iNorm);
						if (TexCoords.size() > iTex)
						{
							vertex.Tex = TexCoords.at(iTex);
						}
						else
						{
							vertex.Tex.x = 0.0f;
							vertex.Tex.y = 0.0f;
						}

						if (bIsSkinned)
						{
							skinVertex = SetupSkinVertex(&mMesh, pSkinnedMesh, iPos);
							skinVertex.Position = vertex.Position;
							skinVertex.Normal = vertex.Normal;
							skinVertex.Tex = vertex.Tex;
						}

						// add the vertex if not already present
						UINT16 index = AddVertex(&mMesh, &vertex, &skinVertex, bIsSkinned);
						// set the index
						mMesh.m_Indices.push_back(index);
					}
					if (f % 10 == 0)
					{
						GetCOREInterface()->ProgressUpdate(100*(f+1)/NumFaces);
					}
				}
				subset.IndexCount = mMesh.m_Indices.size();
				UINT16 id = AddSubset(&subset);
				mMesh.m_Subsets.push_back(id);
				GetCOREInterface()->ProgressEnd();
			}
			else
			{
				Subsets = 0;
				Tab <int> SubMaterials = pMesh->GetActiveMatIDs();
				UINT16 nSubMaterials = SubMaterials.Count();
				UINT16 i = 0;
				for( int m=0; m<nSubMaterials; m++)
				{
					DX_SUBSET subset;
					IGameMaterial* iMaterial;
					DX_MATERIAL dxMat;

					iMaterial = nodeMaterial->GetSubMaterial(m);
					dxMat = ConvertIGameMaterial(iMaterial);
					SETNAME(subset.Name, dxMat.Name);
					subset.MaterialID = AddMaterial(&dxMat);

					char title[200];
					sprintf_s(title, 200, "%s,%s", name, subset.Name);
					GetCOREInterface()->ProgressStart(title, TRUE, fn, NULL);

					// collect the indices
					Tab <FaceEx*> faces = pMesh->GetFacesFromMatID(m);
					NumFaces = faces.Count();
					UINT16 iCount=0;
					subset.IndexStart = i;
					for (UINT16 f=0; f<NumFaces; f++)
					{
						DX_VERTEX vertex;
						DX_VERTEX_SKIN skinVertex;

						FaceEx* face;
						// get the face
						face = faces[f];

						UINT iPos, iNorm, iTex;
						for(int i=0; i<3; i++)
						{
							iPos = face->vert[i];
							iNorm = face->norm[i];
							iTex = face->texCoord[i];

							vertex.Position = Positions.at(iPos);
							vertex.Normal = Normals.at(iNorm);
							vertex.Tex = TexCoords.at(iTex);

							if (bIsSkinned)
							{
								skinVertex = SetupSkinVertex(&mMesh, pSkinnedMesh, iPos);
								skinVertex.Position = vertex.Position;
								skinVertex.Normal = vertex.Normal;
								skinVertex.Tex = vertex.Tex;
							}

							UINT16 index = AddVertex(&mMesh, &vertex, &skinVertex, bIsSkinned);
							mMesh.m_Indices.push_back(index);
						}
						if (f % 10 == 0)
						{
							GetCOREInterface()->ProgressUpdate(100*(f+1)/NumFaces);
						}
					}
					GetCOREInterface()->ProgressEnd();

					subset.IndexCount = (UINT16)mMesh.m_Indices.size() - i;
					i = (UINT16)mMesh.m_Indices.size();
					if (NumFaces > 0)
					{
						Subsets++;
						UINT16 id = AddSubset(&subset);
						mMesh.m_Subsets.push_back(id);
					}
				}
			}

			DX_VERTEX_BUFFER_HEADER vbh;
			vbh.NumVertices = bIsSkinned ? mMesh.m_SkinnedVertices.size() : mMesh.m_Vertices.size();
			vbh.StrideBytes = bIsSkinned ? sizeof(DX_VERTEX_SKIN) : sizeof(DX_VERTEX);
			vbh.SizeBytes = vbh.NumVertices * vbh.StrideBytes;
			vbh.Skinned = bIsSkinned;

			DX_INDEX_BUFFER_HEADER ibh;
			ibh.NumIndices = mMesh.m_Indices.size();
			ibh.SizeBytes = ibh.NumIndices * sizeof(UINT16);

			mMesh.m_Mesh.VertexBuffer = AddVertexBuffer(&vbh);
			mMesh.m_Mesh.IndexBuffer = AddIndexBuffer(&ibh);
			mMesh.m_Mesh.NumSubsets = Subsets;

			UINT16 meshID = m_Meshes.size();
			m_Meshes.push_back(mMesh);

			if (bIsSkinned)
			{
				m_BonesList.push_back( &m_Meshes.at(meshID).m_Bones );
			}

			return meshID;
		}
		else
		{
			char title[200];
			sprintf_s(title, 200, "Collison Mesh: %s", name);
			GetCOREInterface()->ProgressStart(title, TRUE, fn, NULL);

			UINT16 NumVertices = pMesh->GetNumberOfVerts();
			UINT16 NumFaces = pMesh->GetNumberOfFaces();
			int NumTotal = NumVertices + NumFaces;
			int done = 0;
			
			GMatrix gMatWorldInverse;
			gMatWorldInverse = (*mWorld);
			gMatWorldInverse = gMatWorldInverse.Inverse();
			Point3 local;

			// collect vertices
			for (UINT16 v=0; v<NumVertices; v++)
			{
				Point3 point = pMesh->GetVertex(v);
				point.x /= m_Scale;
				point.y /= m_Scale;
				point.z /= m_Scale;
				Point3 transformedPoint = point * gMatWorldInverse;
				P3 p = { transformedPoint.x, transformedPoint.y, transformedPoint.z };
				mMesh.m_CollisionPoints.push_back(p);

				done++;
				if (done % 10 == 0)
				{
					GetCOREInterface()->ProgressUpdate((done*100)/NumTotal);
				}
			}

			// add the indices
			for (UINT16 f=0; f<NumFaces; f++)
			{
				FaceEx* face = pMesh->GetFace(f);
				mMesh.m_Indices.push_back(face->vert[0]);
				mMesh.m_Indices.push_back(face->vert[1]);
				mMesh.m_Indices.push_back(face->vert[2]);

				done++;
				if (done % 10 == 0)
				{
					GetCOREInterface()->ProgressUpdate((done*100)/NumTotal);
				}
			}

			DX_VERTEX_BUFFER_HEADER vbh;
			vbh.NumVertices = mMesh.m_CollisionPoints.size();
			vbh.StrideBytes = sizeof(P3);
			vbh.SizeBytes = vbh.NumVertices * vbh.StrideBytes;
			vbh.Skinned = false;

			DX_INDEX_BUFFER_HEADER ibh;
			ibh.NumIndices = mMesh.m_Indices.size();
			ibh.SizeBytes = ibh.NumIndices * sizeof(UINT16);

			mMesh.m_Mesh.VertexBuffer = AddVertexBuffer(&vbh);
			mMesh.m_Mesh.IndexBuffer = AddIndexBuffer(&ibh);
			mMesh.m_Mesh.NumSubsets = 1;
			DX_SUBSET subset;
			SETNAME(subset.Name, name);
			subset.MaterialID = 0;
			subset.IndexStart = 0;
			subset.IndexCount = mMesh.m_Indices.size();
			mMesh.m_Subsets.push_back( AddSubset(&subset) );

			UINT16 meshID = m_Meshes.size();
			m_Meshes.push_back(mMesh);

			GetCOREInterface()->ProgressEnd();
			return meshID;
		}
	}
	

	return INVALID_VALUE;
}







// set the sibling param of the children frames of pFrame accordingly
void DXExporter::SetFrameChildSiblings(UINT16 pFrame)
{
	UINT16 ParentFrame = pFrame;
	UINT16 numChildren;
	UINT16* Children;

	numChildren = GetNumChildren(ParentFrame);

	if (numChildren == 0)
	{
		return;
	}

	Children = new UINT16[numChildren];
	GetChildren(ParentFrame, Children);


	for( UINT16 i=0; i<numChildren; i++)
	{
		if (i < numChildren - 1)
		{
			m_Frames.at(Children[i]).SiblingFrame = Children[i+1];
		}
		else
		{
			m_Frames.at(Children[i]).SiblingFrame = INVALID_VALUE;
		}
	}
}








void DXExporter::SetupAnimation(IGameNode* pNode, IGameControl* pNodeControl, AnimInfo* pAnimInfo, UINT AnimDataId)
{
	IGameControl* pControl = pNodeControl;
	IGameKeyTab sampledKeys;

	if (pControl == 0x000)
	{
		pControl = pNode->GetIGameControl();
	}

	char* nodeName = pNode->GetName();

	if (pNode->GetIGameObject()->GetIGameType() == IGameObject::IGAME_MESH)
	{
		bool posAnim = pControl->IsAnimated(IGAME_POS);
		bool rotAnim = pControl->IsAnimated(IGAME_ROT);
		if (!posAnim && !rotAnim)
		{
			return;
		}
	}

	pControl->GetFullSampledKeys(sampledKeys, pAnimInfo->Step, IGAME_TM, true);
	int sampledKeysCount = sampledKeys.Count();

	UINT start = pAnimInfo->Start;
	UINT end = pAnimInfo->End;
	UINT frameRate = (UINT)pAnimInfo->FPS;
	UINT numFrames = end - start;
	DX_ANIMATION_FRAME animationFrame;
	UINT numKeys = 0;


	DX_ANIMATION_DATA* AnimationData = &m_AnimationData.at(AnimDataId);


	char title[200];
	sprintf_s(title, 200, "Animation:%s", nodeName);
	GetCOREInterface()->ProgressStart(title, TRUE, fn, NULL);

	// gather keys
	std::vector<UINT> animationFramesList;

	for(int i=0; i<sampledKeysCount; i++)
	{
		UINT iFrame = TicksToSec(sampledKeys[i].t) * frameRate;
		if (iFrame >= start && iFrame <= end)
		{
			AddFrameNumber(&animationFramesList, iFrame);
		}
	}
	numKeys = animationFramesList.size();


	for (UINT i=0; i<numKeys; i++)
	{
		UINT iFrame = animationFramesList.at(i);
		pIgame->SetStaticFrame(iFrame);
		
		GMatrix gMat = pNode->GetLocalTM();
		
		D3DXMATRIX dMat;
		ConvertGToDMatrix(&gMat, &dMat);
		dMat._41 /= m_Scale;
		dMat._42 /= m_Scale;
		dMat._43 /= m_Scale;
		
		DX_ANIMATION_KEY key;
		KEY keyData;
		
		D3DXVECTOR3 vPos, vScale;
		D3DXQUATERNION qRot;
		
		D3DXMatrixDecompose(&vScale, &qRot, &vPos, &dMat);
		
		D3DXQuaternionInverse(&qRot, &qRot);

		P3 pos = {vPos.x, vPos.y, vPos.z};
		P4 rot = {qRot.x, qRot.y, qRot.z, qRot.w};
		P3 scale = {vScale.x, vScale.y, vScale.z};
		
		keyData.Position = pos;
		keyData.Rotation = rot;
		keyData.Scale = scale;
		key.Data = keyData;
		key.Frame = iFrame;
		m_AnimationData.at(AnimDataId).keys.push_back(key);

		if (i % 15 == 0)
		{
			GetCOREInterface()->ProgressUpdate(100*(i+1)/numKeys);
		}
	}

	animationFrame.NumKeys = numKeys;
	animationFrame.FirstKey = AnimationData->keys.size() - animationFrame.NumKeys;

	// correct the rotation keys
	// see 'Notes' DXHeaders.h for detail
	for (int i=1; i<numKeys; i++)
	{
		int curKeyId = animationFrame.FirstKey + i;
		int prevKeyId = animationFrame.FirstKey + i - 1;
		KEY* pCurKey = &AnimationData->keys.at(curKeyId).Data;
		KEY* pPrevKey = &AnimationData->keys.at(prevKeyId).Data;

		P4 curRotPt = pCurKey->Rotation;
		P4 prevRotPt = pPrevKey->Rotation;

		D3DXQUATERNION curRot(curRotPt.x, curRotPt.y, curRotPt.z, curRotPt.w);
		D3DXQUATERNION prevRot(prevRotPt.x, prevRotPt.y, prevRotPt.z, prevRotPt.w);

		float dot = D3DXQuaternionDot(&curRot, &prevRot);
		if (dot < 0.0f)
		{
			curRot = -curRot;
			P4 newCurRot = { curRot.x, curRot.y, curRot.z, curRot.w };
			pCurKey->Rotation = newCurRot;
		}
	}

	animationFrame.Frame = FindFrame(nodeName);
	AnimationData->header.NumFrames++;
	AnimationData->header.NumAnimationKeys += animationFrame.NumKeys;
	AnimationData->header.AnimationDataSize += animationFrame.NumKeys * StructKeySize;
	AnimationData->header.FrameDataSize = AnimationData->header.NumFrames * StructAnimationFrameSize;
	AnimationData->header.FrameDataOffset = StructAnimationHeaderSize;
	AnimationData->header.AnimationDataOffset = AnimationData->header.FrameDataOffset + AnimationData->header.FrameDataSize;


	AnimationData->frames.push_back(animationFrame);

	animationFramesList.clear();

	GetCOREInterface()->ProgressEnd();
}









void DXExporter::SetupBoneFrames(CMesh* pMesh, IGameSkin* pSkinnedMesh)
{
	std::vector <DX_FRAME> BoneFrames;
	std::vector <DX_BONE> Bones;
	std::vector <DX_BONE>* AdjustedBones = &pMesh->m_Bones;

	int TotalBones = pSkinnedMesh->GetTotalSkinBoneCount();

	for (int i=0; i<TotalBones; i++)
	{
		AddSingleBoneFromSkinnedMeshToBoneList(pSkinnedMesh, &Bones, pMesh, i);
	}


	for (int b=0; b<1; b++)
	{
		for (int i=0; i<TotalBones; i++)
		{
			DX_BONE* curBone = &Bones.at(i);

			if (FindBoneId(&Bones, curBone->ParentId) == INVALID_VALUE && curBone->ParentId != 0)
			{
				DX_BONE newBone;
				IGameNode* newNode = pIgame->GetIGameNode(curBone->ParentId);
				newBone = SetupBoneStruct(newNode);
				AddBoneToList(&Bones, &newBone);
			}

			curBone = &Bones.at(i);

			if (FindBoneId(&Bones, curBone->ChildId) == INVALID_VALUE)
			{
				DX_BONE newBone;
				IGameNode* newNode = pIgame->GetIGameNode(curBone->ChildId);
				newBone = SetupBoneStruct(newNode);
				AddBoneToList(&Bones, &newBone);
			}
		}
		TotalBones = Bones.size();
	}


	UINT16 lowestParentId = INVALID_VALUE;
	UINT16 lowestParentIndex = 0;
	UINT16 b;
	for (b=0; b<TotalBones; b++)
	{
		UINT16 id = Bones.at(b).ParentId;
		if (id < lowestParentId)
		{
			lowestParentId = id;
			lowestParentIndex = Bones.at(b).Id;
		}
	}

	PutBonesInOrder(AdjustedBones, &Bones, lowestParentIndex);


	pMesh->m_Mesh.FirstBoneFrame = m_Frames.size();
	pMesh->m_Mesh.NumBones = TotalBones;

	for (UINT16 i=0; i<TotalBones; i++)
	{
		DX_BONE* curBone;
		DX_FRAME curFrame;
		curBone = &AdjustedBones->at(i);
		
		SETNAME(curFrame.Name, curBone->Name);
		curFrame.ChildFrame = INVALID_VALUE;
		curFrame.ParentFrame = INVALID_VALUE;
		curFrame.SiblingFrame = INVALID_VALUE;
		curFrame.Mesh = INVALID_VALUE;
		curFrame.Matrix = curBone->XForm;
		AddFrame(&curFrame);
	}



	for (UINT16 i=0; i<TotalBones; i++)
	{
		DX_BONE* curBone;
		DX_FRAME* curFrame;
		curBone = &AdjustedBones->at(i);
		curFrame = &m_Frames.at(FindFrame(curBone->Name));
		
		DX_BONE *childBone, *parentBone, *siblingBone;

		childBone = FindBone(AdjustedBones, curBone->ChildId);
		parentBone = FindBone(AdjustedBones, curBone->ParentId);
		siblingBone = FindBone(AdjustedBones, curBone->SiblingId);

		if (childBone)
		{
			curFrame->ChildFrame = FindFrame(childBone->Name);
		}

		if (siblingBone)
		{
			curFrame->SiblingFrame = FindFrame(siblingBone->Name);
		}

		if (parentBone)
		{
			curFrame->ParentFrame = FindFrame(parentBone->Name);
		}
		else if (curBone->ParentId == 0)
		{
			curFrame->ParentFrame = 0;
		}
	}
}







// setups a frame and it's child frames
UINT16 DXExporter::SetupFrame(IGameNode* pNode, UINT16 ParentFrame, bool AnimationOnly, AnimInfo* pAnimInfo, UINT AnimDataId)
{
	IGameObject* pObject = pNode->GetIGameObject();

	char* nodeName = pNode->GetName();
	IGameObject::ObjectTypes objectType = pObject->GetIGameType();
	IGameGenObject* genObj = (IGameGenObject*)pObject;
	IPropertyContainer* pC = genObj->GetIPropertyContainer();
	int nProps = pC->GetNumberOfProperties();
	
	INode* pMaxNode = pNode->GetMaxNode();
	BOOL paramExportMesh = FALSE;
	BOOL paramCollisionMesh = FALSE;
	pMaxNode->GetUserPropBool("ExportMesh", paramExportMesh);
	pMaxNode->GetUserPropBool("CollisionMesh", paramCollisionMesh);

	DX_FRAME CurrentFrame;
	SETNAME(CurrentFrame.Name, nodeName);

	if(!AnimationOnly && objectType == IGameObject::IGAME_MESH)
	{
		GMatrix mat = pNode->GetLocalTM();
		ConvertGToDMatrix(&mat, &CurrentFrame.Matrix);
		CurrentFrame.Matrix._41 /= m_Scale;
		CurrentFrame.Matrix._42 /= m_Scale;
		CurrentFrame.Matrix._43 /= m_Scale;
		CurrentFrame.ParentFrame = ParentFrame;
		CurrentFrame.ChildFrame = INVALID_VALUE;
		CurrentFrame.SiblingFrame = INVALID_VALUE;
		CurrentFrame.Mesh = INVALID_VALUE;

		GMatrix gmatWorld = pNode->GetWorldTM();
		gmatWorld[3][0] /= m_Scale;
		gmatWorld[3][1] /= m_Scale;
		gmatWorld[3][2] /= m_Scale;

		if (pObject->GetIGameType() == IGameObject::IGAME_MESH && paramExportMesh == TRUE)
		{
			CurrentFrame.Mesh = SetupMesh(pNode, &gmatWorld, paramCollisionMesh);
		}
		AddFrame(&CurrentFrame);
	}
	
	if (pNode->IsGroupOwner() || pNode->GetChildCount() > 0 )
	{
		int nChildren = pNode->GetChildCount();
		UINT16 ParentId = FindFrame( CurrentFrame.Name );
		if (ParentId != INVALID_VALUE)
		{
			for (int i=0; i<nChildren; i++)
			{
				IGameNode* cNode = pNode->GetNodeChild(i);
				UINT16 cID = SetupFrame(cNode, ParentId, AnimationOnly, pAnimInfo, AnimDataId );
				if (i==0 && !AnimationOnly)
				{
					DX_FRAME* pFrame = &m_Frames.at(ParentId);
					pFrame->ChildFrame = cID;
				}
			}
		}
	}


	if (AnimationOnly)
	{
		UINT16 frameId = FindFrame(CurrentFrame.Name, ParentFrame);
		if (frameId == INVALID_VALUE)
		{
			return INVALID_VALUE;
		}
		IGameControl* pControl = pNode->GetIGameControl();
		SetupAnimation(pNode, pControl, pAnimInfo, AnimDataId);
	}

	return FindFrame(CurrentFrame.Name, ParentFrame);
}








DX_BONE DXExporter::SetupBoneStruct(IGameNode* pBoneNode)
{
	DX_BONE curBone;

	IGameNode* pParentNode = pBoneNode->GetNodeParent();
	UINT numChild = pBoneNode->GetChildCount();

	char* boneName = pBoneNode->GetName();
	SETNAME(curBone.Name, boneName);
	curBone.Id = pBoneNode->GetNodeID();

	SETNAME(curBone.Sibling, "");
	curBone.SiblingId = INVALID_VALUE;

	if (pParentNode)
	{
		char* parentName = pParentNode->GetName();
		SETNAME(curBone.Parent, parentName);
		curBone.ParentId = pParentNode->GetNodeID();
		UINT16 numParentChild = pParentNode->GetChildCount();
		if (numParentChild > 1)
		{
			for (UINT16 i=0; i<numParentChild; i++)
			{
				UINT16 pcId = pParentNode->GetNodeChild(i)->GetNodeID();
				if (curBone.Id == pcId)
				{
					if (i < numParentChild - 1)
					{
						IGameNode* pSibling = pParentNode->GetNodeChild(i+1);
						SETNAME(curBone.Sibling, pSibling->GetName());
						curBone.SiblingId = pSibling->GetNodeID();
					}
					else
					{
						SETNAME(curBone.Sibling, "");
						curBone.SiblingId = INVALID_VALUE;
					}
				}
			}
		}
	}
	else
	{
		SETNAME(curBone.Parent, ROOT_FRAME_NAME);
		curBone.ParentId = 0;
	}

	if (numChild > 0)
	{
		IGameNode* pChildNode = pBoneNode->GetNodeChild(0);
		char* childName = pChildNode->GetName();
		SETNAME(curBone.Child, childName);
		curBone.ChildId = pChildNode->GetNodeID();
	}
	else
	{
		SETNAME(curBone.Child, "");
		curBone.ChildId = INVALID_VALUE;
	}

	GMatrix gBoneXform;
	D3DXMATRIX d3dBoneXform;
	
	pIgame->SetStaticFrame(0);
	gBoneXform = pBoneNode->GetLocalTM();
	ConvertGToDMatrix(&gBoneXform, &d3dBoneXform);
	d3dBoneXform._41 /= m_Scale;
	d3dBoneXform._42 /= m_Scale;
	d3dBoneXform._43 /= m_Scale;
	curBone.XForm = d3dBoneXform;

	return curBone;
}










// The export procedure

int	DXExporter::DoExport(const TCHAR* name,ExpInterface* ei,Interface* i, BOOL suppressPrompts, DWORD options)
{
	// This is where the file export operation occur.

	if(!suppressPrompts)
		DialogBoxParam(hInstance, 
				MAKEINTRESOURCE(IDD_PANEL), 
				GetActiveWindow(), 
				DXExporterOptionsDlgProc, (LPARAM)this);


	// Construct a tab with all this scene's nodes.
	// We could also only take the node currently selected, etc.
	INodeTab lNodes;
	GetSceneNodes(lNodes);


	if (!Export)
	{
		goto quit;
	}


	// Initialise 3DXI (formerly IGame) object
	// For more information, please see 3ds Max SDK topic PG: Programming with 3DXI.
	IGameConversionManager* cm = GetConversionManager();
	cm->SetCoordSystem(IGameConversionManager::IGAME_D3D);
	
	bool bSelected = ( options & SCENE_EXPORT_SELECTED ) ? true : false;
	pIgame = GetIGameInterface();	
	pIgame->InitialiseIGame(bSelected);
	pIgame->SetStaticFrame(0);



	int nNode = pIgame->GetTopLevelNodeCount();
	for (int i=0; i<nNode; i++)
	{
		IGameNode* pNode = pIgame->GetTopLevelNode(i);
		SetupFrame(pNode, 0, false, NULL, NULL);
	}




	if (bExportAnimation)
	{
		for (int i=0; i<Animations.size(); i++)
		{
			DX_ANIMATION_HEADER header;
			header.Version = DX_FILE_VERSION;
			header.NumFrames = 0;
			header.NumAnimationKeys = 0;
			header.AnimationDataSize = 0;
			header.AnimationDataOffset = 0;

			AnimInfo* pInfo = &Animations.at(i);
			SETNAME(header.Name, pInfo->Name);
			header.AnimationFPS = pInfo->FPS;
			
			DX_ANIMATION_DATA AnimData;
			AnimData.header = header;

			UINT16 animationId = m_AnimationData.size();

			m_AnimationData.push_back(AnimData);

			for (int i=0; i<nNode; i++)
			{
				IGameNode* pNode = pIgame->GetTopLevelNode(i);
				SetupFrame(pNode, 0, true, pInfo, animationId);
			}
		}
	}



	UINT16 NumberOfMeshes = m_Meshes.size();
	UINT16 NumberOfSubsets = m_Subsets.size();
	UINT16 NumberOfFrames = m_Frames.size();
	UINT16 NumberOfMaterials = m_Materials.size();
	UINT16 NumberOfVbs = m_VertexBuffers.size();
	UINT16 NumberOfIbs = m_IndexBuffers.size();
	UINT16 NumberOfAnimations = m_AnimationData.size();


	for (int i=0; i<NumberOfFrames; i++)
	{
		SetFrameChildSiblings(i);
	}


	FileHeader.NumFrames = NumberOfFrames;
	FileHeader.NumMeshes = NumberOfMeshes;
	FileHeader.NumTotalSubsets = NumberOfSubsets;
	FileHeader.NumMaterials = NumberOfMaterials;
	FileHeader.NumVertexBuffers = NumberOfVbs;
	FileHeader.NumIndexBuffers = NumberOfIbs;

	UINT64 FrameBytes = StructFrameSize * NumberOfFrames;
	UINT64 MeshBytes = StructMeshSize * NumberOfMeshes;
	UINT64 SubsetBytes = StructSubsetSize * NumberOfSubsets;
	UINT64 MaterialBytes = StructMaterialSize * NumberOfMaterials;
	UINT64 VBBytes = StructVBSize * NumberOfVbs;
	UINT64 IBBytes = StructIBSize * NumberOfIbs;
	UINT64 SubsetBufferDataBytes = 0;
	UINT64 VertexBufferDataBytes = 0;
	UINT64 IndexBufferDataBytes = 0;
	

	FileHeader.NonBufferDataSize = FrameBytes + MeshBytes + SubsetBytes +
		MaterialBytes + VBBytes + IBBytes;

	FileHeader.FrameDataOffset = StructHeaderSize;
	FileHeader.MeshDataOffset = FileHeader.FrameDataOffset + FrameBytes;
	FileHeader.SubsetDataOffset = FileHeader.MeshDataOffset + MeshBytes;
	FileHeader.MaterialDataOffset = FileHeader.SubsetDataOffset + SubsetBytes;
	FileHeader.VertexDataOffset = FileHeader.MaterialDataOffset + MaterialBytes;
	FileHeader.IndexDataOffset = FileHeader.VertexDataOffset + VBBytes;
	FileHeader.BufferDataOffset = FileHeader.HeaderSize + FileHeader.NonBufferDataSize;


	for(UINT16 m=0; m<NumberOfMeshes; m++)
	{
		SubsetBufferDataBytes += m_Meshes.at(m).m_Mesh.NumSubsets * sizeof(UINT16);
	}

	for(UINT16 v=0; v<NumberOfVbs; v++)
	{
		VertexBufferDataBytes += m_VertexBuffers.at(v).SizeBytes;
	}

	for(UINT16 i=0; i<NumberOfIbs; i++)
	{
		IndexBufferDataBytes += m_IndexBuffers.at(i).SizeBytes;
	}

	FileHeader.BufferDataSize = SubsetBufferDataBytes + VertexBufferDataBytes +
		IndexBufferDataBytes;


	// ---------------------- WRITING PROCESS BEGINS HERE -------------------------

	char title[200] = "Writing...";

	Interface* exp = GetCOREInterface();
	exp->ProgressStart(title, TRUE, fn, NULL);
	UINT64 TotalDataSize = FileHeader.HeaderSize + FileHeader.NonBufferDataSize + FileHeader.BufferDataSize;
	UINT64 BytesWritten = 0;


	// open our file
	fopen_s(&m_File, name, "wb");


	// Write the file header
	fwrite(&FileHeader, StructHeaderSize, 1, m_File);
	BytesWritten += StructHeaderSize;
	exp->ProgressUpdate(1);



	for(UINT16 i=0; i<NumberOfFrames; i++)
	{
		fwrite(&m_Frames.at(i), StructFrameSize, 1, m_File);
		BytesWritten += StructFrameSize;

	}
	exp->ProgressUpdate(10);


	for(UINT16 i=0; i<NumberOfMeshes; i++)
	{
		DX_MESH* pMesh = &m_Meshes.at(i).m_Mesh;
		if (i==0)
		{
			pMesh->SubsetOffset = 0; 
		}
		else
		{
			DX_MESH* pPrevMesh = &m_Meshes.at(i-1).m_Mesh;
			UINT64 SubsetsSize = pPrevMesh->NumSubsets * sizeof(UINT16);
			(pMesh->SubsetOffset) = pPrevMesh->SubsetOffset + SubsetsSize;
		}
		fwrite(pMesh, StructMeshSize, 1, m_File);
		BytesWritten += StructMeshSize;
	}
	exp->ProgressUpdate(12);



	for(UINT16 i=0; i<NumberOfSubsets; i++)
	{
		DX_SUBSET* pSubset = &m_Subsets.at(i);
		fwrite(pSubset, StructSubsetSize, 1, m_File);
		BytesWritten += StructSubsetSize;
	}
	exp->ProgressUpdate(14);



	for(UINT16 i=0; i<NumberOfMaterials; i++)
	{
		DX_MATERIAL* pMaterial = &m_Materials.at(i);
		fwrite(pMaterial, StructMaterialSize, 1, m_File);
		BytesWritten += StructMaterialSize;
	}
	exp->ProgressUpdate(16);



	for(UINT16 i=0; i<NumberOfVbs; i++)
	{
		DX_VERTEX_BUFFER_HEADER* pVB = &m_VertexBuffers.at(i);
		if (i==0)
		{
			pVB->DataOffset = SubsetBufferDataBytes;
		}
		else
		{
			DX_VERTEX_BUFFER_HEADER* pPrevVB = &m_VertexBuffers.at(i-1);
			UINT64 offset = pPrevVB->SizeBytes;
			pVB->DataOffset = pPrevVB->DataOffset + offset;
		}
		fwrite(pVB, StructVBSize, 1, m_File);
		BytesWritten += StructVBSize;
	}
	exp->ProgressUpdate(18);




	for(UINT16 i=0; i<NumberOfIbs; i++)
	{
		DX_INDEX_BUFFER_HEADER* pIB = &m_IndexBuffers.at(i);
		if (i==0)
		{
			pIB->DataOffset = SubsetBufferDataBytes + VertexBufferDataBytes;
		}
		else
		{
			DX_INDEX_BUFFER_HEADER* pPrevIB = &m_IndexBuffers.at(i-1);
			UINT64 offset = pPrevIB->SizeBytes;
			pIB->DataOffset = pPrevIB->DataOffset + offset;
		}
		fwrite(pIB, StructIBSize, 1, m_File);
		BytesWritten += StructIBSize;
	}
	exp->ProgressUpdate(20);



	fflush(m_File);


	for(UINT16 m=0; m<NumberOfMeshes; m++)
	{
		CMesh* pMesh = &m_Meshes.at(m);
		UINT16 NumSubsets = pMesh->m_Subsets.size();
		for(UINT16 s=0; s<NumSubsets; s++)
		{
			fwrite(&pMesh->m_Subsets.at(s), sizeof(UINT16), 1, m_File);
			BytesWritten += sizeof(UINT16);
		}
	}
	exp->ProgressUpdate(24);

	fflush(m_File);

	
	for(UINT16 m=0; m<NumberOfMeshes; m++)
	{
		CMesh* pMesh = &m_Meshes.at(m);
		DX_VERTEX_BUFFER_HEADER* vbh = &m_VertexBuffers.at(pMesh->m_Mesh.VertexBuffer);
		UINT16 Stride = vbh->StrideBytes;
		bool bSkinned = vbh->Skinned;
		UINT16 NumVerts = vbh->NumVertices;
		if (bSkinned)
		{
			for(UINT16 v=0; v<NumVerts; v++)
			{
				fwrite(&pMesh->m_SkinnedVertices.at(v), Stride, 1, m_File);
				BytesWritten += Stride;
			}
		}
		else
		{
			if (pMesh->bCollision)
			{
				for(UINT16 v=0; v<NumVerts; v++)
				{
					fwrite(&pMesh->m_CollisionPoints.at(v), Stride, 1, m_File);
					BytesWritten += Stride;
				}
			}
			else
			{
				for(UINT16 v=0; v<NumVerts; v++)
				{
					fwrite(&pMesh->m_Vertices.at(v), Stride, 1, m_File);
					BytesWritten += Stride;
				}
			}
		}
	}
	exp->ProgressUpdate(60);

	fflush(m_File);

	for(UINT16 m=0; m<NumberOfMeshes; m++)
	{
		CMesh* pMesh = &m_Meshes.at(m);
		UINT16 NumIndices = pMesh->m_Indices.size();
		for(UINT16 i=0; i<NumIndices; i++)
		{
			fwrite(&pMesh->m_Indices.at(i), sizeof(UINT16), 1, m_File);
			BytesWritten += sizeof(UINT16);
		}
	}
	exp->ProgressUpdate(80);

	fclose(m_File);



	for(UINT16 a=0; a<NumberOfAnimations; a++)
	{
		DX_ANIMATION_DATA* pAnimData = &m_AnimationData.at(a);
		DX_ANIMATION_HEADER* pHeader = &pAnimData->header;

		char filePath[MAX_PATH];
		strcpy_s(filePath, MAX_PATH, name);
		char* pLastSlash = strrchr(filePath, '.');
		*pLastSlash = NULL;
		strcat_s(filePath, MAX_PATH, "_");
		strcat_s(filePath, MAX_PATH, pHeader->Name);
		strcat_s(filePath, MAX_PATH, ".dxanim");

		FILE* m_AnimFile;
		fopen_s(&m_AnimFile, filePath, "wb");

		fwrite(pHeader, StructAnimationHeaderSize, 1, m_AnimFile);
		BytesWritten += StructAnimationHeaderSize;

		for(UINT32 f=0; f<pHeader->NumFrames; f++)
		{
			DX_ANIMATION_FRAME* frame = &pAnimData->frames.at(f);
			fwrite(frame, StructAnimationFrameSize, 1, m_AnimFile);
			BytesWritten += StructAnimationFrameSize;
		}

		fflush(m_AnimFile);

		for(UINT32 k=0; k<pHeader->NumAnimationKeys; k++)
		{
			DX_ANIMATION_KEY* key = &pAnimData->keys.at(k);
			fwrite(key, StructKeySize, 1, m_AnimFile);
			BytesWritten += StructKeySize;
		}
		fclose(m_AnimFile);
	}

	exp->ProgressUpdate(100);
	exp->ProgressEnd();

	// ---------------------- WRITING PROCESS ENDS HERE -------------------------

	Animations.clear();
	MessageBox(NULL, "Export complete!", "DXExporter", MB_OK|MB_ICONINFORMATION);


quit:

	return TRUE;
}


 
