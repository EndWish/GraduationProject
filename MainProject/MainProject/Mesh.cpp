#include "stdafx.h"
#include "Mesh.h"

///////////////////////////////////////////////////////////////////////////////
/// Mesh

/// ��� ������
// ������ �� �Ҹ���
Mesh::Mesh() {
	m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_nVertices = 0;
	m_pPositionBuffer.Reset();
	m_positionBufferView = D3D12_VERTEX_BUFFER_VIEW();

	m_OOBB = BoundingOrientedBox();
}
Mesh::~Mesh()
{

}

/// ��� �Լ���
// ������
void Mesh::Render(ID3D12GraphicsCommandList* pCommandList) 
{
	pCommandList->IASetPrimitiveTopology(m_primitiveTopology);
	pCommandList->IASetVertexBuffers(0, 1, &m_positionBufferView);
	pCommandList->DrawInstanced(m_nVertices, 1, 0, 0);

}


///////////////////////////////////////////////////////////////////////////////
/// IndexingMesh

/// ��� ������
// ������ �� �Ҹ���
IndexingMesh::IndexingMesh()
{

}
IndexingMesh::~IndexingMesh()
{

}

/// ��� �Լ���
// ������
void IndexingMesh::Render(ID3D12GraphicsCommandList* pCommandList) {
	pCommandList->IASetPrimitiveTopology(m_primitiveTopology);
	pCommandList->IASetVertexBuffers(0, 1, &m_positionBufferView);

	for (int subSet = 0; subSet < m_pSubSetIndexBuffers.size(); ++subSet) {
		// ������ ���� ���� [����]
		pCommandList->IASetIndexBuffer(&m_subSetIndexBufferViews[subSet]);
		pCommandList->DrawIndexedInstanced(m_subSetIndices[subSet], 1, 0, 0, 0);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// NormalIndexingMesh
NormalIndexingMesh::NormalIndexingMesh() 
{
	m_pNormalBuffer.Reset();
	m_normalBufferView = D3D12_VERTEX_BUFFER_VIEW();
}
NormalIndexingMesh::~NormalIndexingMesh() 
{

}

/// ��� �Լ���
// ������
void NormalIndexingMesh::Render(ID3D12GraphicsCommandList* pCommandList) {
	pCommandList->IASetPrimitiveTopology(m_primitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_positionBufferView, m_normalBufferView };
	pCommandList->IASetVertexBuffers(0, 2, pVertexBufferViews);

	for (int subSet = 0; subSet < m_pSubSetIndexBuffers.size(); ++subSet) {
		// ������ ���� ���� [����]
		pCommandList->IASetIndexBuffer(&m_subSetIndexBufferViews[subSet]);
		pCommandList->DrawIndexedInstanced(m_subSetIndices[subSet], 1, 0, 0, 0);
	}
}
