#include "stdafx.h"
#include "Button.h"
#include "GameFramework.h"
Button::Button(string _imgName, XMFLOAT2 _size, XMFLOAT2 _position, ButtonType _type, const ComPtr<ID3D12Device>& _pDevice, const ComPtr<ID3D12GraphicsCommandList>& _pCommandList) {
	
	// 인자를 줄때 편하게 하기 위해 클라이언트 창의 left, top부분을 0,0, 반대편을 2,2 좌표 기준
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
	// 해당 위치에서 눌렀다 떼는 행동을 한 경우 버튼을 플레이어가 누른것으로 판단
	if (isClick && !_isClick && CheckClick(_pos)) act = true;
	isClick = _isClick;
	return act;
}

void Button::Render(const ComPtr<ID3D12GraphicsCommandList>& _pCommandList)
{
	if(enable) img->Render(_pCommandList);
}
