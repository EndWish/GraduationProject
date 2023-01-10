#pragma once
#include "Image2D.h"

enum class ButtonType : unsigned char {
	start,	// ���� ��ư
	room,	// �� ����� �����ִ� ��ư
	exit,	// ���� ��ư
	option,
};

class Button
{
private:
	bool enable;	// ��ư�� Ȱ��ȭ �� �������� Ȯ��
	bool isClick;	// ���� �� ��ư�� ���� �������� Ȯ��
	ButtonType type;	// ��ư�� Ÿ��
	shared_ptr<Image2D> img;	// ��ư�� �׸� �̹���
	XMFLOAT2 position;	// ��ư�� ����Ʈ ��ǥ�� ���� ��ġ
	XMFLOAT2 size;	// ��ư�� ũ��
public:
	Button(string _imgName, XMFLOAT2 _size, XMFLOAT2 _position, ButtonType _type, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
	~Button();
	bool CheckClick(XMFLOAT2 _pos);
	bool GetIsClick() { return isClick; };
	void SetEnable(bool _enable) { enable = _enable; if (!_enable) isClick = false; };
	bool CheckEnable();
	bool Press(bool _isClick, XMFLOAT2 _pos);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList);
};

