# GraduationProject  
## 개요  
한국공학대학교 졸업작품(2018180009 김시인 / 2018180010 김연규 / 2018182034 전수민)  
기간 : 2022.08 ~ 20.23.08  
개발 환경 : C++, DirectX12, Visual Studio, 3DsMax, ZBrush  

### 연구목적 :
  1. 공간 분할, 프러스텀 컬링, 하드웨어 인스턴싱 등 최적화에 대한 여러 기법을 적용해보며 효율적인 게임을 만들기 위한 능력을 개발한다.  
  2. 모델링, 텍스처 및 애니메이션 제작을 통해 실무 능력을 배양한다.  
  3. 각종 그래픽스 기술을 적용해보며 쉐이더 프로그래밍 및 DirectX12 파이프라인에 대한 이해도를 높인다.  

## 게임 소개  
장르 : 비대칭 멀티 서바이멀 게임  
플레이 인원 : 학생 플레이어 2~4명, 교수 플레이어 1명  
Dead By DayLight를 모티브로 하여 실제 한국공학대학교 E공학관을 배경으로 학생과 교수의 숨바꼭질 탈출게임  

#### 게임 흐름
![image](https://github.com/EndWish/GraduationProject/assets/90082921/afe6f9a0-c067-48c9-b389-6c0ec61b1e90)  

![image](https://github.com/EndWish/GraduationProject/assets/90082921/4a46a61a-bf5c-40ce-b155-4b10f3011f70)  

#### 게임 조작
![image](https://github.com/EndWish/GraduationProject/assets/90082921/c137b5e8-57ed-4f15-8077-44c6cf0d4565)  

## 개발 내용  
1. 교수 플레이어의 근접, 투사체 공격 구현 :  
![그림1](https://github.com/EndWish/GraduationProject/assets/90082921/d89463d7-9f0e-477d-a8f8-5afda7ceaab5)  
![그림2](https://github.com/EndWish/GraduationProject/assets/90082921/bbcfb69d-a316-4db7-b4cf-e4248bf694c9)  

2. 학생 플레이어의 투명스킬과 아이템 구현 :  
![그림3](https://github.com/EndWish/GraduationProject/assets/90082921/c421b91a-b6c5-4be7-9d0d-1b9f9822fd0b)  
![image](https://github.com/EndWish/GraduationProject/assets/90082921/aba51614-1278-4ba3-976c-70ccfaacc30c)  

3. 상황에 따른 스키닝 애니메이션 구현 및 슬라이딩 벡터를 이용한 충동구현 :  
![Animation원본](https://github.com/EndWish/GraduationProject/assets/90082921/379bc360-a9ed-4f00-a710-43b818d3b109)  

4. SO 단계 및 기하 쉐이더를 사용한 파티클 구현 :  
![image](https://github.com/EndWish/GraduationProject/assets/90082921/32fdb229-8789-40b0-8e82-16b7519c50c6)  
![image](https://github.com/EndWish/GraduationProject/assets/90082921/cc71f0c0-e15d-41e6-bf0e-8570e344bbc0)  
![image](https://github.com/EndWish/GraduationProject/assets/90082921/1afd35ba-dc69-497c-a4f2-c5488080b057)  
![image](https://github.com/EndWish/GraduationProject/assets/90082921/5b68f9a1-2fc2-48c6-9263-fa1a9fec825b)  

5. 조명과 공격에 이미시브를 구현한여 적용 :  
![image](https://github.com/EndWish/GraduationProject/assets/90082921/1642e60c-c7c2-4a3d-8892-5bae8edf65d1)  

6. 노멀맵 적용 :  
![image](https://github.com/EndWish/GraduationProject/assets/90082921/cc2071c2-e03b-4df3-84c5-57c52aa5ba1b)  

7. 가려지는 부분에 따른 카메라 거리 조절 :  
![Camera Move](https://github.com/EndWish/GraduationProject/assets/90082921/e5224905-d506-45bb-bef5-0e5dac3c227e)  

8. 많은 그림자 및 조명 처리를 위한 지연 조명, 쉐도우 맵 사용 :  
![image](https://github.com/EndWish/GraduationProject/assets/90082921/06b73d8f-2711-47e3-b928-8ade02c3ac87)  

9. 공간 분할 및 뷰 프러스텀 컬링으로 렌더링 및 충돌 처리 최적화, 하드웨어 인스턴싱으로 렌더링 최적화  
![image](https://github.com/EndWish/GraduationProject/assets/90082921/bd60b9db-427e-4758-9b21-f623e8feec01)  

10. 교수 플레이어의 레이더 스킬 추가 :  
![Rayder 원본 (3) (1)](https://github.com/EndWish/GraduationProject/assets/90082921/a1f5da55-b9a2-486b-9ea5-47deb3627b79)  

11. 정적인 물체에 대해서 쉐도우맵 베이킹하여 움직이는 물체에 대해서만 동적으로 쉐도우맵을 만들고 두 쉐도우맵을 병합하여 성능 향상  
12. AsyncSelect모델을 이용한 멀티 플레이 구현  
13. 사운드를 제외한 모델, 애니메이션, UI 텍스처 등 모든 리소스 자체 제작  
14. 외곽선 구현
15. 반투명 오브젝트 구현  
16. 로비 씬 및 닉네임 구현  
17. 사운드 구현  

## 최종 발표 결과  
한국공학대전 / 전체 3등(총장상) / 2023.09  

