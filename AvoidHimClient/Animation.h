#pragma once

class GameObject;

class AnimationClip {
protected:
	string name;
	float runTime;
	UINT nKeyFrame;
	bool isLoop;
	float speedRatio;

	vector<vector<XMFLOAT3>> scales, position;
	vector<vector<XMFLOAT4>> rotation;

public:
	AnimationClip();
	~AnimationClip();

	const string& GetName() const { return name; }
	float GetRunTime() const { return runTime; }
	UINT GetNKeyFrame() const { return nKeyFrame; }
	XMFLOAT3 GetScale(int boneIndex, int keyFrameIndex) const { return scales[boneIndex][keyFrameIndex]; }
	XMFLOAT3 GetPosition(int boneIndex, int keyFrameIndex) const { return position[boneIndex][keyFrameIndex]; }
	XMFLOAT4 GetRotation(int boneIndex, int keyFrameIndex) const { return rotation[boneIndex][keyFrameIndex]; }

	bool IsLoop() const { return isLoop; }
	void SetLoop(bool _isLoop) { isLoop = _isLoop; }

	float getSpeedRatio() const { return speedRatio; }
	void setSpeedRatio(float _speedRatio) { speedRatio = _speedRatio; }

	void LoadFromFile(ifstream& _file, UINT _nBone);
	
};

class AnimationController {
protected:
	UINT nBone;
	string currentAniClipName;
	unordered_map<string, shared_ptr<AnimationClip>> pAniClips;
	float time;
	int nRepeat;

public:
	AnimationController();
	~AnimationController();

	void AddTime(float _time);
	void LoadFromFile(ifstream& _file, UINT _nBone);
	void UpdateBoneLocalTransform(vector<shared_ptr<GameObject>>& _pBones);
	void ChangeClip(const string& _name, float _time = 0);

	const string& GetClipName() const { return currentAniClipName; }
	float GetTime() const { return time; }
	void SetTime(float _time) { time = _time; }

	int GetNRepeat() const { return nRepeat; }
	void SetNRepeat(int _nRepeat) { nRepeat = _nRepeat; }

	bool IsMaxFrame() const;

};
