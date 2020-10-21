#include "assetManager.h"
#include "../mesh/box.h"
#include "../mesh/sphere.h"
#include "../mesh/board.h"
#include "../mesh/quad.h"
#include "../mesh/terrain.h"
#include "../constants/constants.h"
using namespace std;

AssetManager* AssetManager::assetManager = NULL;

AssetManager::AssetManager() {
	texBld = new TextureBindless();
	skyTexture = NULL;
	envTexture = NULL;
	noise3DTexture = NULL;
	reflectTexture = NULL;
	heightTexture = NULL;
	distortionTex = -1;
	noiseTex = -1;
	roadTex = -1;
	meshes.clear();
	animations.clear();
	frames = new FrameMgr();
}

AssetManager::~AssetManager() {
	map<string, Mesh*>::iterator itor;
	for (itor = meshes.begin(); itor != meshes.end(); itor++)
		delete itor->second;
	meshes.clear();
	map<string, Animation*>::iterator iter;
	for (iter = animations.begin(); iter != animations.end(); iter++)
		delete iter->second;
	animations.clear();
	delete frames;
	if (texBld) delete texBld; texBld = NULL;
	if (heightTexture) delete heightTexture; heightTexture = NULL;
	if (skyTexture) delete skyTexture;
	if (envTexture && envTexture != skyTexture) delete envTexture;
	skyTexture = NULL; envTexture = NULL;
	if (noise3DTexture) delete noise3DTexture; noise3DTexture = NULL;
}

void AssetManager::addTextureBindless(const char* name, bool srgb, int wrap) {
	texBld->addTexture(name, srgb, wrap);
}

void AssetManager::initTextureBindless(MaterialManager* mtls) {
	for (uint i = 0; i < mtls->size(); i++) {
		Material* mat = mtls->find(i);
		if (mat->prepared) continue;
		if (mat->tex1.length() > 0) {
			if (texBld->findTexture(mat->tex1.data()) < 0) texBld->addTexture(mat->tex1.data(), mat->srgb1);
			mat->texids.x = texBld->findTexture(mat->tex1.data());
		}
		if (mat->tex2.length() > 0) {
			if (texBld->findTexture(mat->tex2.data()) < 0) texBld->addTexture(mat->tex2.data(), mat->srgb2);
			mat->texids.y = texBld->findTexture(mat->tex2.data());
		}
		if (mat->tex3.length() > 0) {
			if (texBld->findTexture(mat->tex3.data()) < 0) texBld->addTexture(mat->tex3.data(), mat->srgb3);
			mat->texids.z = texBld->findTexture(mat->tex3.data());
		}
		if (mat->tex4.length() > 0) {
			if (texBld->findTexture(mat->tex4.data()) < 0) texBld->addTexture(mat->tex4.data(), mat->srgb4);
			mat->texids.w = texBld->findTexture(mat->tex4.data());
		}
		printf("mat %s: [%d]%s\n", mat->name.data(), (int)mat->texids.x, mat->tex1.data());
	}
	texBld->initData(COMMON_TEXTURE);
}

int AssetManager::findTextureBindless(const char* name) {
	return texBld->findTexture(name);
}

void AssetManager::setSkyTexture(CubeMap* tex) {
	if (tex == skyTexture) return;
	if (skyTexture) delete skyTexture;
	skyTexture = tex;
}

CubeMap* AssetManager::getSkyTexture() {
	return skyTexture;
}

void AssetManager::setEnvTexture(CubeMap* tex) {
	if (tex == envTexture) return;
	if (envTexture) delete envTexture;
	envTexture = tex;
}

CubeMap* AssetManager::getEnvTexture() {
	return envTexture;
}

void AssetManager::setNoise3D(CubeMap* tex) {
	if (tex == noise3DTexture) return;
	if (noise3DTexture) delete noise3DTexture;
	noise3DTexture = tex;
}

CubeMap* AssetManager::getNoise3D() {
	return noise3DTexture;
}

void AssetManager::setReflectTexture(Texture2D* tex) {
	reflectTexture = tex;
}

Texture2D* AssetManager::getReflectTexture() {
	return reflectTexture;
}

void AssetManager::addDistortionTex(const char* texName) {
	if (distortionTex < 0)
		addTextureBindless(texName, false);
	distortionTex = findTextureBindless(texName);
}

void AssetManager::addNoiseTex(const char* texName) {
	if (noiseTex < 0)
		addTextureBindless(texName, false, WRAP_MIRROR);
	noiseTex = findTextureBindless(texName);
}

void AssetManager::addRoadTex(const char* texName) {
	if (roadTex < 0)
		addTextureBindless(texName, false);
	roadTex = findTextureBindless(texName);
}

void AssetManager::createHeightTex() {
	if (meshes.count("terrain") <= 0) return;
	Terrain* mesh = (Terrain*)meshes["terrain"];
	byte* heightData = mesh->getHeightMap();
	heightTexture = new Texture2D(MAP_SIZE, MAP_SIZE, TEXTURE_TYPE_COLOR, HIGH_PRE, 1, LINEAR, true, heightData);
}

void AssetManager::addMesh(const char* name, Mesh* mesh, bool billboard, bool drawShadow) {
	mesh->setName(name);
	meshes[name] = mesh;
	meshes[name]->setIsBillboard(billboard);
	meshes[name]->drawShadow = drawShadow;
}

void AssetManager::addAnimation(const char* name, Animation* animation) {
	animations[name] = animation;
	animation->setName(name);
	frames->addAnimation(animation);
}

void AssetManager::initFrames() {
	frames->init();
}

void AssetManager::Init() {
	if (!AssetManager::assetManager) {
		InitImageLoaders(); // Init freeimage
		AssetManager::assetManager = new AssetManager();

		// Load some basic meshes
		AssetManager::assetManager->addMesh("box", new Box());
		AssetManager::assetManager->addMesh("sphere", new Sphere(16, 16));
		AssetManager::assetManager->addMesh("board", new Board());
		AssetManager::assetManager->addMesh("quad", new Quad());
		AssetManager::assetManager->addMesh("billboard", new Board(1, 1, 1, 0, 0), true, false);
	}
}

void AssetManager::Release() {
	if (AssetManager::assetManager) {
		ReleaseImageLoaders(); // Release freeimage
		delete AssetManager::assetManager;
	}
	AssetManager::assetManager = NULL;
}