#include	"Field.h"
#include	"DebugUI.h"

using namespace DirectX::SimpleMath;


void Field::Init()
{
	// �����b�V������
	m_PlaneMesh.Init(
		60, 60,						// ������
		960,						// �T�C�YX
		960,						// �T�C�YZ
		Color(1, 1, 1, 1),			// ���_�J���[
		Vector3(0, 1, 0),			// �@���x�N�g��
		Vector3(1, 0, 0),			// �ڃx�N�g��
		true);						// XZ����

	// �����_���[������
	m_MeshRenderer.Init(m_PlaneMesh);

	// �}�e���A������
	MATERIAL	mtrl;
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1, 1, 1, 1);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;

	m_Material.Create(mtrl);

	// �s��̏�����
	m_WorldMatrix.Mat = Matrix::Identity;
	m_WorldMatrix.InvMat = Matrix::Identity;
	m_WorldMatrix.PrevMat = Matrix::Identity;

	// �e�N�X�`�����[�h
	bool sts = m_Texture.Load("assets\\texture\\tile_diffuse.png");
	assert(sts == true);
	sts = m_Normal.Load("assets\\texture\\tile_normal.png");
	assert(sts == true);
}


void Field::Draw()
{
	// �s�����O�t���[���ɕۑ�
	m_WorldMatrix.PrevMat = m_WorldMatrix.Mat;

	// SRT���쐬
	Matrix r = 
		Matrix::CreateFromYawPitchRoll(
			m_Rotation.y, 
			m_Rotation.x, 
			m_Rotation.z);

	Matrix t = Matrix::CreateTranslation(
		m_Position.x, 
		m_Position.y, 
		m_Position.z);

	Matrix s = Matrix::CreateScale(
		m_Scale.x, 
		m_Scale.y, 
		m_Scale.z);

	// ���[���h�s����Z�b�g
	m_WorldMatrix.Mat = s * r * t;

	// �t�s������߂�
	m_WorldMatrix.InvMat = m_WorldMatrix.Mat.Invert();

	Renderer::SetWorldMatrix(&m_WorldMatrix);		// GPU�ɃZ�b�g

	auto num = m_PlaneMesh.GetIndices().size() / 3;
	// �`�����Applicaion�Ɋi�[
	Application::CountDrawInfo(num);

	m_Material.SetGPU();
	m_Texture.SetGPU(DIFFUSE);
	m_Normal.SetGPU(NORMAL);
	m_MeshRenderer.Draw();
}

void Field::Dispose()
{

}


