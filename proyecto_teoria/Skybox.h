#pragma once
// Skybox + cubemap de ambiente (T4, T10). El cubemap se genera por codigo (cielo crepuscular sci-fi:
// degradado, nebulosa, estrellas, luna y planeta con anillo). Se usa para (1) dibujar el fondo y
// (2) reflejo/refraccion en materiales metalicos y de cristal (environment mapping + Fresnel).
// Para usar fotos reales: sustituye CrearCubemap() por 6 imagenes con SOIL_load_OGL_cubemap.
#include <vector>
#include <cmath>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"

namespace skybox_detail
{
	inline float hash3(glm::vec3 p)
	{
		float h = std::sin(glm::dot(p, glm::vec3(127.1f, 311.7f, 74.7f))) * 43758.5453f;
		return h - std::floor(h);
	}
	inline float noise3(glm::vec3 p)   // ruido de valor trilineal
	{
		glm::vec3 i = glm::floor(p), f = p - i;
		f = f * f * (glm::vec3(3.0f) - 2.0f * f);
		auto h = [&](float x, float y, float z) { return hash3(i + glm::vec3(x, y, z)); };
		float a = glm::mix(glm::mix(h(0,0,0), h(1,0,0), f.x), glm::mix(h(0,1,0), h(1,1,0), f.x), f.y);
		float b = glm::mix(glm::mix(h(0,0,1), h(1,0,1), f.x), glm::mix(h(0,1,1), h(1,1,1), f.x), f.y);
		return glm::mix(a, b, f.z);
	}
	inline float fbm(glm::vec3 p)
	{
		float s = 0.0f, a = 0.5f;
		for (int i = 0; i < 5; i++) { s += a * noise3(p); p *= 2.03f; a *= 0.5f; }
		return s;
	}

	// color del cielo en la direccion d (espacio mundo, +Y arriba)
	inline glm::vec3 Cielo(glm::vec3 d)
	{
		d = glm::normalize(d);
		float h = d.y;
		glm::vec3 zenit(0.02f, 0.04f, 0.14f), medio(0.16f, 0.12f, 0.36f), horizonte(0.95f, 0.45f, 0.30f);
		glm::vec3 c;
		if (h >= 0.0f) {
			c = glm::mix(horizonte, medio, glm::smoothstep(0.0f, 0.25f, h));
			c = glm::mix(c, zenit, glm::smoothstep(0.2f, 0.9f, h));
		} else {   // bajo el horizonte: oscuro, para que el reflejo del suelo no se vea blanco
			c = glm::mix(horizonte * 0.35f, glm::vec3(0.02f, 0.02f, 0.05f), glm::smoothstep(0.0f, 0.5f, -h));
		}
		// nebulosa verde-magenta
		float n = fbm(d * 3.0f + glm::vec3(5.0f));
		float neb = glm::smoothstep(0.50f, 0.85f, n) * glm::smoothstep(0.05f, 0.4f, h);
		c += neb * glm::mix(glm::vec3(0.10f, 0.45f, 0.30f), glm::vec3(0.55f, 0.15f, 0.45f), fbm(d * 2.0f)) * 0.55f;
		// estrellas
		if (h > 0.0f) {
			glm::vec3 q = d * 90.0f;
			float e = hash3(glm::floor(q));
			if (e > 0.985f) {
				glm::vec3 fr = q - glm::floor(q) - glm::vec3(0.5f);
				float br = (e - 0.985f) / 0.015f * glm::smoothstep(0.35f, 0.0f, glm::length(fr));
				c += glm::vec3(br) * glm::smoothstep(0.0f, 0.3f, h);
			}
		}
		// luna
		glm::vec3 luna = glm::normalize(glm::vec3(-0.5f, 0.45f, -0.7f));
		float dl = std::acos(glm::clamp(glm::dot(d, luna), -1.0f, 1.0f));
		c += glm::vec3(1.0f, 0.97f, 0.88f) * glm::smoothstep(0.075f, 0.065f, dl) * (0.75f + 0.25f * fbm(d * 40.0f));
		c += glm::vec3(1.0f, 0.8f, 0.6f) * std::exp(-dl * 9.0f) * 0.35f;   // halo
		// planeta con anillo
		glm::vec3 pl = glm::normalize(glm::vec3(0.7f, 0.35f, 0.6f));
		float dp = std::acos(glm::clamp(glm::dot(d, pl), -1.0f, 1.0f));
		if (dp < 0.16f) {
			float b = 0.5f + 0.5f * std::sin(d.y * 90.0f + 4.0f * fbm(d * 10.0f));
			c = glm::mix(c, glm::mix(glm::vec3(0.55f, 0.25f, 0.15f), glm::vec3(0.85f, 0.6f, 0.4f), b) * (0.4f + 0.6f * glm::clamp(dot(d - pl, glm::vec3(-0.5f, 0.8f, 0.3f)) * 6.0f + 0.5f, 0.0f, 1.0f)), glm::smoothstep(0.16f, 0.15f, dp));
		}
		float ring = std::abs(dp - 0.26f);
		if (ring < 0.05f && std::abs(glm::dot(d - pl * glm::dot(d, pl), glm::normalize(glm::cross(pl, glm::vec3(0, 1, 0))))) > 0.0f) {
			float a = glm::smoothstep(0.05f, 0.0f, ring) * (0.4f + 0.6f * noise3(glm::vec3(dp * 400.0f)));
			c = glm::mix(c, glm::vec3(0.8f, 0.7f, 0.55f), a * 0.55f);
		}
		return c;
	}
}

// Genera el cubemap (6 caras, 'n' x 'n') y regresa su id
inline GLuint CrearCubemap(int n = 512)
{
	using namespace skybox_detail;
	GLuint id; glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	std::vector<unsigned char> px(n * n * 3);
	for (int f = 0; f < 6; f++)
	{
		for (int y = 0; y < n; y++)
			for (int x = 0; x < n; x++)
			{
				float u = 2.0f * (x + 0.5f) / n - 1.0f, v = 2.0f * (y + 0.5f) / n - 1.0f;
				glm::vec3 d;
				switch (f) {
					case 0: d = glm::vec3( 1.0f, -v, -u); break;   // +X
					case 1: d = glm::vec3(-1.0f, -v,  u); break;   // -X
					case 2: d = glm::vec3(   u,  1.0f,  v); break; // +Y
					case 3: d = glm::vec3(   u, -1.0f, -v); break; // -Y
					case 4: d = glm::vec3(   u,  -v,  1.0f); break;// +Z
					default: d = glm::vec3( -u,  -v, -1.0f); break;// -Z
				}
				glm::vec3 c = glm::clamp(Cielo(d), 0.0f, 1.0f);
				unsigned char* o = &px[(y * n + x) * 3];
				for (int k = 0; k < 3; k++) o[k] = (unsigned char)(std::pow(c[k], 1.0f / 2.2f) * 255.0f);   // sRGB aprox.
			}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GL_RGB, n, n, 0, GL_RGB, GL_UNSIGNED_BYTE, px.data());
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return id;
}

// Cubo unitario para dibujar el fondo
struct Skybox
{
	GLuint vao = 0, vbo = 0;
	void Init()
	{
		static const float v[] = {
			-1,1,-1, -1,-1,-1, 1,-1,-1, 1,-1,-1, 1,1,-1, -1,1,-1,
			-1,-1,1, -1,-1,-1, -1,1,-1, -1,1,-1, -1,1,1, -1,-1,1,
			1,-1,-1, 1,-1,1, 1,1,1, 1,1,1, 1,1,-1, 1,-1,-1,
			-1,-1,1, -1,1,1, 1,1,1, 1,1,1, 1,-1,1, -1,-1,1,
			-1,1,-1, 1,1,-1, 1,1,1, 1,1,1, -1,1,1, -1,1,-1,
			-1,-1,-1, -1,-1,1, 1,-1,-1, 1,-1,-1, -1,-1,1, 1,-1,1 };
		glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glBindVertexArray(0);
	}
	// 'view' normal; se le quita la traslacion para que el cielo siempre quede al infinito
	void Draw(Shader& sh, GLuint cubemap, const glm::mat4& view, const glm::mat4& proj)
	{
		glDepthFunc(GL_LEQUAL);
		sh.Use();
		glm::mat4 v = glm::mat4(glm::mat3(view));
		glUniformMatrix4fv(glGetUniformLocation(sh.Program, "view"), 1, GL_FALSE, &v[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(sh.Program, "projection"), 1, GL_FALSE, &proj[0][0]);
		glUniform1i(glGetUniformLocation(sh.Program, "skybox"), 10);
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
		glDepthFunc(GL_LESS);
	}
};
