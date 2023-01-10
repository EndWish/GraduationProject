#pragma once
#include "Image2D.h"

enum class ButtonType : unsigned char {
	start,	// 시작 버튼
	room,	// 방 목록을 보여주는 버튼
	exit,	// 종료 버튼
	option,
};

class Button
{
private:
	bool enable;	// 버튼이 활성화 된 상태인지 확인
	bool isClick;	// 현재 이 버튼이 눌린 상태인지 확인
	ButtonType type;	// 버튼의 타입
	shared_ptr<Image2D> img;	// 버튼을 그릴 이미지
	XMFLOAT2 position;	// 버튼의 뷰포트 좌표계 기준 위치
	XMFLOAT2 size;	// 버튼의 크기
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

