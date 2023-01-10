#include "stdafx.h"
#include "Button.h"
#include "GameFramework.h"
Button::Button(string _imgName, XMFLOAT2 _size, XMFLOAT2 _position, ButtonType _type, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	// ���ڸ� �ٶ� ���ϰ� �ϱ� ���� Ŭ���̾�Ʈ â�� left, top�κ��� 0,0, �ݴ����� 2,2 ��ǥ ����
	GameFramework& gameFramework = GameFramework::Instance();
	auto [width, height] = gameFramework.GetClientSize();
	position = _position;
	size = _size;
	type = _type;
	enable = true;
	isClick = false;
	img = make_shared<Image2D>(_imgName, size, position, XMFLOAT2(1, 1), _pDevice, _pCommandList);
	
}

Button::~Button() {

}

bool Button::CheckClick(XMFLOAT2 _pos)
{
	return _pos.x >= (position.x-1) && _pos.x <= (position.x-1) + size.x && _pos.y >= (position.y-1) && _pos.y <= (position.y-1) + size.y;
}

bool Button::CheckEnable()
{
	return enable;
}

bool Button::Press(bool _isClick, XMFLOAT2 _pos)
{
	bool act = false;
	img->SetPress(_isClick);
	// �ش� ��ġ���� ������ ���� �ൿ�� �� ��� ��ư�� �÷��̾ ���������� �Ǵ�
	if (isClick && !_isClick && CheckClick(_pos)) act = true;
	isClick = _isClick;
	return act;
}

void Button::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)
{
	if(enable) img->Render(_pCommandList);
}
