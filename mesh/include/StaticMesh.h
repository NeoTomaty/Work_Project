#pragma once

#include	<simplemath.h>
#include	<string>
#include	<vector>
#include	<memory>
#include	"Texture.h"
#include	"Mesh.h"
#include	"Renderer.h"

class StaticMesh : public Mesh {
public:
	void Load(std::string filename);

	void LoadTex(std::string filename, TexType type)
	{
		switch (type)
		{
		case DIFFUSE:
			m_albedomap.Load(filename);
			break;
		case NORMAL:
			m_normalmap.Load(filename);
			break;
		}
	}

	void SetTexture()
	{
		m_albedomap.SetGPU(DIFFUSE);
		m_normalmap.SetGPU(NORMAL);
	}

	const std::vector<MATERIAL>& GetMaterials() {
		return m_materials;
	}

	const std::vector<SUBSET>& GetSubsets() {
		return m_subsets;
	}

	const std::vector<std::string>& GetDiffuseTextureNames() {
		return m_diffusetexturenames;
	}


private:

	std::vector<MATERIAL> m_materials;					// �}�e���A�����
	std::vector<std::string> m_diffusetexturenames;		// �f�B�t���[�Y�e�N�X�`����
	std::vector<SUBSET> m_subsets;						// �T�u�Z�b�g���

	Texture	m_albedomap;	// �f�B�t���[�Y�}�b�v
	Texture	m_normalmap;	// �@���}�b�v

};