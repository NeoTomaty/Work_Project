#include	"Polygon2D.h"
#include	"Application.h"

using namespace DirectX::SimpleMath;

void Polygon2D::Init()
{
	// ���_�f�[�^
	std::vector<VERTEX_3D>	vertices;

	auto maxsizeX = Application::GetWidth();
	auto maxsizeY = Application::GetHeight();

	vertices.resize(4);

	vertices[0].Position = Vector3(0, 0, 0);				// ����
	vertices[1].Position = Vector3(maxsizeX, 0, 0);			// �E��
	vertices[2].Position = Vector3(0, maxsizeY, 0);			// ����
	vertices[3].Position = Vector3(maxsizeX, maxsizeY, 0);	// �E��

	vertices[0].TexCoord = Vector2(0, 0);
	vertices[1].TexCoord = Vector2(1, 0);
	vertices[2].TexCoord = Vector2(0, 1);
	vertices[3].TexCoord = Vector2(1, 1);

	// ���_�o�b�t�@����
	m_VertexBuffer.Create(vertices);

	// �C���f�b�N�X�o�b�t�@����
	std::vector<unsigned int> indices;
	indices.resize(4);

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 3;

	// �C���f�b�N�X�o�b�t�@����
	m_IndexBuffer.Create(indices);

	// �}�e���A������
	MATERIAL	mtrl;
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1, 1, 1, 1);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;


	m_Material.Create(mtrl);

}

void Polygon2D::Update()
{
	// ���_�f�[�^
	std::vector<VERTEX_3D>	vertices;

	vertices.resize(4);

	auto maxsizeX = Application::GetWidth();
	auto maxsizeY = Application::GetHeight();

	vertices[0].Position = Vector3(0, 0, 0);				// ����
	vertices[1].Position = Vector3(maxsizeX, 0, 0);			// �E��
	vertices[2].Position = Vector3(0, maxsizeY, 0);			// ����
	vertices[3].Position = Vector3(maxsizeX, maxsizeY, 0);	// �E��

	vertices[0].TexCoord = Vector2(0, 0);
	vertices[1].TexCoord = Vector2(1, 0);
	vertices[2].TexCoord = Vector2(0, 1);
	vertices[3].TexCoord = Vector2(1, 1);

	m_VertexBuffer.Modify(vertices);

}

void Polygon2D::Draw()
{
	// 2D�`��p
	Renderer::SetWorldViewProjection2D();

	ID3D11DeviceContext* devicecontext;

	devicecontext = Renderer::GetDeviceContext();

	// �g�|���W�[���Z�b�g�i���v���~�e�B�u�^�C�v�j
	devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	m_VertexBuffer.SetGPU();
	m_IndexBuffer.SetGPU();

	m_Material.SetGPU();

	devicecontext->DrawIndexed(
		4,				// �`�悷��C���f�b�N�X���i�l�p�`�Ȃ̂łS�j
		0,				// �ŏ��̃C���f�b�N�X�o�b�t�@�̈ʒu
		0);
}

void Polygon2D::Dispose()
{

}