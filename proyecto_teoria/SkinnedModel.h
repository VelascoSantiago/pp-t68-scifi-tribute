#pragma once
// Skinning por huesos (T5): carga un glTF/FBX con Assimp (malla + esqueleto + animaciones de Blender/Mixamo),
// evalua la animacion (keyframes de huesos con interpolacion) y manda las matrices de huesos al shader (lighting.vs).
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Model.h"

class SkinnedModel
{
public:
	static const int MAX_HUESOS = 100;

	bool Load(const std::string& archivo, const std::string& dirTexturas)
	{
		sc = imp.ReadFile(archivo, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_LimitBoneWeights | aiProcess_JoinIdenticalVertices);
		if (!sc || !sc->mRootNode) { std::cout << "ERROR::ASSIMP:: " << imp.GetErrorString() << std::endl; return false; }
		texDir = dirTexturas;
		for (unsigned i = 0; i < sc->mNumMeshes; i++) meshes.push_back(CargarMalla(sc->mMeshes[i]));
		for (unsigned a = 0; a < sc->mNumAnimations; a++)
		{
			std::unordered_map<std::string, const aiNodeAnim*> m;
			for (unsigned c = 0; c < sc->mAnimations[a]->mNumChannels; c++) m[sc->mAnimations[a]->mChannels[c]->mNodeName.C_Str()] = sc->mAnimations[a]->mChannels[c];
			canales.push_back(m);
		}
		std::cout << "SkinnedModel: " << meshes.size() << " mallas, " << bones.size() << " huesos, " << sc->mNumAnimations << " animaciones" << std::endl;
		return true;
	}
	int NumAnimaciones() const { return sc ? (int)sc->mNumAnimations : 0; }
	std::string NombreAnimacion(int i) const { return sc->mAnimations[i]->mName.C_Str(); }
	float Duracion(int i) const { const aiAnimation* a = sc->mAnimations[i]; return (float)(a->mDuration / (a->mTicksPerSecond > 0 ? a->mTicksPerSecond : 24.0)); }

	// Animaciones de NODOS (T7: papel picado, petales...): cada nodo con malla se mueve con su propio canal.
	// Todas las animaciones del archivo corren a la vez (el exportador de Blender genera una por objeto).
	void DrawNodos(Shader& sh, const glm::mat4& base, float t)
	{
		if (canalesTodos.empty())
		{
			for (unsigned a = 0; a < sc->mNumAnimations; a++)
			{
				const aiAnimation* an = sc->mAnimations[a];
				tpsTodos = an->mTicksPerSecond > 0 ? an->mTicksPerSecond : 24.0;
				durTodos = std::max(durTodos, an->mDuration);
				for (unsigned c = 0; c < an->mNumChannels; c++) canalesTodos[an->mChannels[c]->mNodeName.C_Str()] = an->mChannels[c];
			}
			canales.push_back(canalesTodos);   // indice 'sc->mNumAnimations' = todas juntas
		}
		GLuint P = sh.Program;
		glUniform1i(glGetUniformLocation(P, "skinned"), 0);
		double tick = durTodos > 0 ? std::fmod(t * tpsTodos, durTodos) : 0.0;
		DibujaNodo(sh, sc->mRootNode, base, (int)canales.size() - 1, tick);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// 'm' = matriz de modelo del personaje; 'anim' = indice de animacion; 't' = segundos (se repite en bucle)
	void Draw(Shader& sh, const glm::mat4& m, int anim, float t)
	{
		GLuint P = sh.Program;
		std::vector<glm::mat4> M(MAX_HUESOS, glm::mat4(1.0f));
		if (anim >= 0 && anim < (int)sc->mNumAnimations)
		{
			const aiAnimation* a = sc->mAnimations[anim];
			double tps = a->mTicksPerSecond > 0 ? a->mTicksPerSecond : 24.0;
			double dur = std::max(a->mDuration, 1.0);
			double tick = std::fmod(t * tps, dur);
			Recorre(sc->mRootNode, glm::mat4(1.0f), anim, tick, M);
		}
		else Recorre(sc->mRootNode, glm::mat4(1.0f), -1, 0.0, M);   // pose de reposo
		glUniformMatrix4fv(glGetUniformLocation(P, "bones"), MAX_HUESOS, GL_FALSE, glm::value_ptr(M[0]));
		glUniform1i(glGetUniformLocation(P, "skinned"), 1);
		glUniformMatrix4fv(glGetUniformLocation(P, "model"), 1, GL_FALSE, glm::value_ptr(m));
		for (SMesh& s : meshes)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, s.tex);
			glUniform1i(glGetUniformLocation(P, "envKind"), s.envKind);
			glBindVertexArray(s.vao);
			glDrawElements(GL_TRIANGLES, (GLsizei)s.n, GL_UNSIGNED_INT, 0);
		}
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUniform1i(glGetUniformLocation(P, "skinned"), 0);
	}

private:
	struct SVert { glm::vec3 p, n; glm::vec2 uv; glm::ivec4 id; glm::vec4 w; };
	struct SMesh { GLuint vao, vbo, ebo; size_t n; GLuint tex; int envKind; };
	Assimp::Importer imp;
	const aiScene* sc = nullptr;
	std::string texDir;
	std::vector<SMesh> meshes;
	std::map<std::string, int> boneIdx;
	std::vector<glm::mat4> offsets;
	std::vector<std::unordered_map<std::string, const aiNodeAnim*>> canales;
	std::map<std::string, GLuint> texCache;
	std::vector<int> bones;   // solo para contar

	static glm::mat4 AGlm(const aiMatrix4x4& a)
	{
		return glm::mat4(a.a1, a.b1, a.c1, a.d1, a.a2, a.b2, a.c2, a.d2, a.a3, a.b3, a.c3, a.d3, a.a4, a.b4, a.c4, a.d4);
	}

	template <class K, class F> static size_t Indice(const K* k, unsigned n, double t, F tiempo)
	{
		for (unsigned i = 0; i + 1 < n; i++) if (t < tiempo(k[i + 1])) return i;
		return n > 1 ? n - 2 : 0;
	}
	glm::mat4 LocalAnimada(const aiNode* nodo, int anim, double t)
	{
		glm::mat4 local = AGlm(nodo->mTransformation);
		if (anim < 0) return local;
		auto it = canales[anim].find(nodo->mName.C_Str());
		if (it == canales[anim].end()) return local;
		const aiNodeAnim* c = it->second;
		// lo que el canal no anima se queda como en el nodo (p. ej. el papel picado solo anima rotacion)
		aiVector3D sc0, p0; aiQuaternion q0; nodo->mTransformation.Decompose(sc0, q0, p0);
		glm::vec3 pos(p0.x, p0.y, p0.z), esc(sc0.x, sc0.y, sc0.z); glm::quat rot(q0.w, q0.x, q0.y, q0.z);
		if (c->mNumPositionKeys) {
			size_t i = Indice(c->mPositionKeys, c->mNumPositionKeys, t, [](const aiVectorKey& k) { return k.mTime; }), j = std::min<size_t>(i + 1, c->mNumPositionKeys - 1);
			double dt = c->mPositionKeys[j].mTime - c->mPositionKeys[i].mTime; float f = dt > 0 ? (float)std::min(1.0, std::max(0.0, (t - c->mPositionKeys[i].mTime) / dt)) : 0.0f;
			const aiVector3D &a = c->mPositionKeys[i].mValue, &b = c->mPositionKeys[j].mValue;
			pos = glm::mix(glm::vec3(a.x, a.y, a.z), glm::vec3(b.x, b.y, b.z), f);
		}
		if (c->mNumRotationKeys) {
			size_t i = Indice(c->mRotationKeys, c->mNumRotationKeys, t, [](const aiQuatKey& k) { return k.mTime; }), j = std::min<size_t>(i + 1, c->mNumRotationKeys - 1);
			double dt = c->mRotationKeys[j].mTime - c->mRotationKeys[i].mTime; float f = dt > 0 ? (float)std::min(1.0, std::max(0.0, (t - c->mRotationKeys[i].mTime) / dt)) : 0.0f;
			const aiQuaternion &a = c->mRotationKeys[i].mValue, &b = c->mRotationKeys[j].mValue;
			rot = glm::normalize(glm::slerp(glm::quat(a.w, a.x, a.y, a.z), glm::quat(b.w, b.x, b.y, b.z), f));
		}
		if (c->mNumScalingKeys) {
			size_t i = Indice(c->mScalingKeys, c->mNumScalingKeys, t, [](const aiVectorKey& k) { return k.mTime; }), j = std::min<size_t>(i + 1, c->mNumScalingKeys - 1);
			double dt = c->mScalingKeys[j].mTime - c->mScalingKeys[i].mTime; float f = dt > 0 ? (float)std::min(1.0, std::max(0.0, (t - c->mScalingKeys[i].mTime) / dt)) : 0.0f;
			const aiVector3D &a = c->mScalingKeys[i].mValue, &b = c->mScalingKeys[j].mValue;
			esc = glm::mix(glm::vec3(a.x, a.y, a.z), glm::vec3(b.x, b.y, b.z), f);
		}
		return glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot) * glm::scale(glm::mat4(1.0f), esc);
	}
	// jerarquia de transformaciones: global(hijo) = global(padre) * local(hijo)
	void Recorre(const aiNode* n, const glm::mat4& padre, int anim, double t, std::vector<glm::mat4>& M)
	{
		glm::mat4 g = padre * LocalAnimada(n, anim, t);
		auto it = boneIdx.find(n->mName.C_Str());
		if (it != boneIdx.end() && it->second < MAX_HUESOS) M[it->second] = g * offsets[it->second];
		for (unsigned i = 0; i < n->mNumChildren; i++) Recorre(n->mChildren[i], g, anim, t, M);
	}

	std::unordered_map<std::string, const aiNodeAnim*> canalesTodos;
	double tpsTodos = 24.0, durTodos = 0.0;

	void DibujaNodo(Shader& sh, const aiNode* n, const glm::mat4& padre, int anim, double t)
	{
		glm::mat4 g = padre * LocalAnimada(n, anim, t);
		GLuint P = sh.Program;
		for (unsigned i = 0; i < n->mNumMeshes; i++)
		{
			SMesh& s = meshes[n->mMeshes[i]];
			glUniformMatrix4fv(glGetUniformLocation(P, "model"), 1, GL_FALSE, glm::value_ptr(g));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, s.tex);
			glUniform1i(glGetUniformLocation(P, "envKind"), 0);
			glBindVertexArray(s.vao);
			glDrawElements(GL_TRIANGLES, (GLsizei)s.n, GL_UNSIGNED_INT, 0);
		}
		for (unsigned i = 0; i < n->mNumChildren; i++) DibujaNodo(sh, n->mChildren[i], g, anim, t);
	}

	GLuint Textura(const std::string& nombre, const aiMaterial* mat)
	{
		std::string base = nombre;
		for (char& c : base) if (c == ' ' || c == '.') c = '_';
		if (texCache.count(base)) return texCache[base];
		GLuint id = 0;
		for (aiTextureType tt : { aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE })
		{
			aiString ap;
			if (!id && mat->GetTextureCount(tt) > 0 && mat->GetTexture(tt, 0, &ap) == AI_SUCCESS && ap.length > 0)
				if (std::ifstream(texDir + "/" + ap.C_Str()).good()) id = TextureFromFile(ap.C_Str(), texDir);
		}
		if (id) return texCache[base] = id;
		for (const char* ext : { ".png", ".jpeg", ".jpg" })
		{
			std::string f = texDir + "/" + base + ext;
			if (std::ifstream(f).good()) { id = TextureFromFile((base + ext).c_str(), texDir); break; }
		}
		if (!id && base == "collar") id = TextureFromFile("images.jpeg", texDir);
		if (!id)
		{
			aiColor4D kd(0.8f, 0.8f, 0.8f, 1.0f);
			aiGetMaterialColor(mat, AI_MATKEY_BASE_COLOR, &kd);
			id = TexturaColorPlano(glm::vec3(kd.r, kd.g, kd.b));   // el Base Color del glTF ya es lineal, igual que espera TexturaColorPlano
		}
		return texCache[base] = id;
	}

	SMesh CargarMalla(const aiMesh* m)
	{
		std::vector<SVert> v(m->mNumVertices);
		for (unsigned i = 0; i < m->mNumVertices; i++)
		{
			v[i].p = glm::vec3(m->mVertices[i].x, m->mVertices[i].y, m->mVertices[i].z);
			v[i].n = m->mNormals ? glm::vec3(m->mNormals[i].x, m->mNormals[i].y, m->mNormals[i].z) : glm::vec3(0, 1, 0);
			v[i].uv = m->mTextureCoords[0] ? glm::vec2(m->mTextureCoords[0][i].x, m->mTextureCoords[0][i].y) : glm::vec2(0.0f);
			v[i].id = glm::ivec4(0); v[i].w = glm::vec4(0.0f);
		}
		for (unsigned b = 0; b < m->mNumBones; b++)
		{
			std::string nom = m->mBones[b]->mName.C_Str();
			int idx;
			if (!boneIdx.count(nom)) { idx = (int)boneIdx.size(); boneIdx[nom] = idx; offsets.push_back(AGlm(m->mBones[b]->mOffsetMatrix)); bones.push_back(idx); }
			else idx = boneIdx[nom];
			for (unsigned w = 0; w < m->mBones[b]->mNumWeights; w++)
			{
				const aiVertexWeight& vw = m->mBones[b]->mWeights[w];
				SVert& s = v[vw.mVertexId];
				for (int k = 0; k < 4; k++) if (s.w[k] == 0.0f) { s.id[k] = idx; s.w[k] = vw.mWeight; break; }
			}
		}
		std::vector<GLuint> ind;
		for (unsigned f = 0; f < m->mNumFaces; f++) for (unsigned k = 0; k < m->mFaces[f].mNumIndices; k++) ind.push_back(m->mFaces[f].mIndices[k]);

		SMesh s{}; s.n = ind.size();
		glGenVertexArrays(1, &s.vao); glGenBuffers(1, &s.vbo); glGenBuffers(1, &s.ebo);
		glBindVertexArray(s.vao);
		glBindBuffer(GL_ARRAY_BUFFER, s.vbo);
		glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(SVert), v.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(GLuint), ind.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SVert), (void*)offsetof(SVert, p));
		glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SVert), (void*)offsetof(SVert, n));
		glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SVert), (void*)offsetof(SVert, uv));
		glEnableVertexAttribArray(3); glVertexAttribIPointer(3, 4, GL_INT, sizeof(SVert), (void*)offsetof(SVert, id));
		glEnableVertexAttribArray(4); glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(SVert), (void*)offsetof(SVert, w));
		glBindVertexArray(0);

		const aiMaterial* mat = sc->mMaterials[m->mMaterialIndex];
		aiString an; mat->Get(AI_MATKEY_NAME, an);
		std::string nom = an.C_Str();
		s.tex = Textura(nom, mat);
		std::string l = nom; for (char& c : l) c = (char)tolower((unsigned char)c);
		auto tiene = [&](const char* x) { return l.find(x) != std::string::npos; };
		s.envKind = (tiene("glass") || tiene("fluor_yellow")) ? 2 : (tiene("rusty") || tiene("plates")) ? 1 : 0;
		return s;
	}
};
