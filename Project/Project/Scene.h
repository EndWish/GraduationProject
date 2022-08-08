#pragma once
class Scene {
protected:

public:
	Scene();
	virtual ~Scene();

	virtual void InitScene();
	virtual void ExitScene();

public:
	virtual void FrameAdvance();
	
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& pCommandList);
};

class PlayScene : public Scene {
private:


public:
	PlayScene();
	~PlayScene() final;

	void InitScene() final;
	void ExitScene() final;

public:
	void FrameAdvance() final;
	void Render(const ComPtr<ID3D12GraphicsCommandList>& pCommandList) final;

};