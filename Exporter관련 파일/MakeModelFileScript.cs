using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.IO;
using System.Text;
using UnityEditor;
using Unity.VisualScripting;



public class MakeModelFileScript : MonoBehaviour
{
    struct SRT
    {
        public Vector3 scale;
        public Quaternion rotation;
        public Vector3 position;
    }

    void BinaryWriteString(string str, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(str.Length);
        binaryWriter.Write(str.ToCharArray());
    }
    void BinaryWriteVector3(Vector3 vector, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(vector.x);
        binaryWriter.Write(vector.y);
        binaryWriter.Write(vector.z);
    }
    void BinaryWriteVector2(Vector2 vector, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(vector.x);
        binaryWriter.Write(vector.y);
    }
    void BinaryWriteQuat(Quaternion vector, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(vector.x);
        binaryWriter.Write(vector.y);
        binaryWriter.Write(vector.z);
        binaryWriter.Write(vector.w);
    }

    void BinaryWriteColor(Color c, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(c.r);
        binaryWriter.Write(c.g);
        binaryWriter.Write(c.b);
        binaryWriter.Write(c.a);
    }
    void BinaryWriteMatrix(Matrix4x4 matrix, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(matrix.m00);
        binaryWriter.Write(matrix.m10);
        binaryWriter.Write(matrix.m20);
        binaryWriter.Write(matrix.m30);
        binaryWriter.Write(matrix.m01);
        binaryWriter.Write(matrix.m11);
        binaryWriter.Write(matrix.m21);
        binaryWriter.Write(matrix.m31);
        binaryWriter.Write(matrix.m02);
        binaryWriter.Write(matrix.m12);
        binaryWriter.Write(matrix.m22);
        binaryWriter.Write(matrix.m32);
        binaryWriter.Write(matrix.m03);
        binaryWriter.Write(matrix.m13);
        binaryWriter.Write(matrix.m23);
        binaryWriter.Write(matrix.m33);
    }
    void BinaryWriteMaterial(Material material, BinaryWriter binaryWriter)
    {
        // albedoNameSize(UINT) / albedoName(string) -> 알베도 텍스처 이름
        // bumpNameSize(UINT) / bumpName(string) -> 노말맵 텍스처 이름
        // 이후 추가
        string[] mapTypes = new string[2]
        {
            "_MainTex",
            "_BumpMap"
        };
        for(int i=0; i<mapTypes.Length; ++i)
        {
            Texture texture = material.GetTexture(mapTypes[i]);

            if (texture)
            {
                BinaryWriteString(texture.name, binaryWriter);
            }
            else
            {
                BinaryWriteString("null", binaryWriter);
            }
        } 
        
        // ambient(XMFLOAT4)
        Color ambient = new Color(0.2f, 0.2f, 0.2f, 1.0f);
        BinaryWriteColor(ambient, binaryWriter);
        // diffuse(XMFLOAT4)
        if (material.HasProperty("_Color"))
        {
            Color diffuse = material.GetColor("_Color");
            BinaryWriteColor(diffuse, binaryWriter);
        }
        else
        {
            Color diffuse = new Color(0.0f, 0.0f, 0.0f, 1.0f);
            BinaryWriteColor(diffuse, binaryWriter);
        }
        // specular(XMFLOAT4)(specular.w = 반짝임계수)
        if (material.HasProperty("_SpecColor"))
        {
            Color specular = material.GetColor("_SpecColor");
            BinaryWriteColor(specular, binaryWriter);
        }
        else
        {
            Color specular = new Color(1.0f, 1.0f, 1.0f, 10.0f);
            BinaryWriteColor(specular, binaryWriter);
        }
        // emissive(XMFLOAT4)
        if (material.HasProperty("_EmissionColor"))
        {
            Color emission = material.GetColor("_EmissionColor");
            BinaryWriteColor(emission, binaryWriter);
        }
        else
        {
            Color emission = new Color(0.0f, 0.0f, 0.0f, 1.0f);
            BinaryWriteColor(emission, binaryWriter);
        }
    }
  
    void CreateMeshBinaryFile(Mesh mesh, ref Bounds modelBound, Vector3 vec, BinaryWriter binaryWriter)
    {

        // init modelbound
        
        Bounds worldBounds = new Bounds(mesh.bounds.center + vec, mesh.bounds.extents * 2);
        modelBound.Encapsulate(worldBounds);

        // nVertex(UINT)
        binaryWriter.Write((uint)mesh.vertexCount);

        // boundingBox (float * 6)
        BinaryWriteVector3(mesh.bounds.center, binaryWriter);
        BinaryWriteVector3(mesh.bounds.extents, binaryWriter);

        // positions (float * 3 * nVertex)
        foreach (Vector3 position in mesh.vertices)
        {
            BinaryWriteVector3(position, binaryWriter);
        }
        // normals (float * 3 * nVertex)
        foreach (Vector3 normal in mesh.normals)
        {
            BinaryWriteVector3(normal, binaryWriter);
        }
        
        // numTexcoord (INT)
        binaryWriter.Write((int)mesh.uv.Length);

        // texcoord (float * 2 * nVertex)
        foreach (Vector2 texcoord in mesh.uv)
        {
            Vector2 tmpuv = texcoord;
            tmpuv.y = 1 - tmpuv.y;  
            BinaryWriteVector2(tmpuv, binaryWriter);
        }

        // nSubMesh (UINT)
        binaryWriter.Write((uint)mesh.subMeshCount);

        for (int i = 0; i < mesh.subMeshCount; i++)
        {
            // nSubMeshIndex (UINT) / subMeshIndex (UINT * nSubMeshIndex)    ....( * nSubMesh )
            int[] subindicies = mesh.GetTriangles(i);
            binaryWriter.Write(subindicies.Length);
            
            foreach (int index in subindicies)
            {
                binaryWriter.Write(index);
            }
        }
    }

    void CreateSkinnedMeshStreamFile(SkinnedMeshRenderer skinnedMeshRenderer, ref Bounds modelBound, Vector3 vec, BinaryWriter binaryWriter)
    {
        // bonesPerVertex(UINT)     //뼈하나당 영향의 주는 정점의 개수를 출력
        uint nBonesPerVertex = 4;
        binaryWriter.Write(nBonesPerVertex);

        // nBone(UINT)		//뼈의 개수
        Transform[] bones = skinnedMeshRenderer.bones;
        binaryWriter.Write(bones.Length);

        // bindposes(float4x4 * nBone)		// 오프셋 행렬들
        Mesh mesh = skinnedMeshRenderer.sharedMesh;
        foreach (Matrix4x4 offsetMatrix in mesh.bindposes)  // 행렬의 개수는 뼈의 개수와 같다.
        {
            BinaryWriteMatrix(offsetMatrix, binaryWriter);
        }

        // skinnedMeshOOBB(float * 6)		// 스킨드 메쉬의 바운딩 박스
        BinaryWriteVector3(skinnedMeshRenderer.localBounds.center, binaryWriter);
        BinaryWriteVector3(skinnedMeshRenderer.localBounds.extents, binaryWriter);

        // 메쉬 정보
        CreateMeshBinaryFile(mesh, ref modelBound, vec, binaryWriter);

        // 각 정점에 영향을 주는 뼈의 인덱스와 가중치
        foreach (BoneWeight boneWeight in mesh.boneWeights)
        {
            // boneIndex(UINT * 4)			// 해당 정점에 영향을 주는 뼈의 인덱스
            binaryWriter.Write((uint)boneWeight.boneIndex0);
            binaryWriter.Write((uint)boneWeight.boneIndex1);
            binaryWriter.Write((uint)boneWeight.boneIndex2);
            binaryWriter.Write((uint)boneWeight.boneIndex3);
        }
        foreach (BoneWeight boneWeight in mesh.boneWeights)
        {
            // boneWeight (float * 4)		// 가중치
            binaryWriter.Write(boneWeight.weight0);
            binaryWriter.Write(boneWeight.weight1);
            binaryWriter.Write(boneWeight.weight2);
            binaryWriter.Write(boneWeight.weight3);
        }

    }

    void CreateObjectBinaryFile(Transform objectTransform, BinaryWriter binaryWriter, ref Bounds modelBound)
    {
        Transform curObjectTransform = objectTransform;

        SkinnedMeshRenderer skinnedMeshRenderer = curObjectTransform.GetComponent<SkinnedMeshRenderer>();
        // IsSkinnedObject(bool)		// 스킨드 오브젝트 인지
        binaryWriter.Write(skinnedMeshRenderer);
        if (skinnedMeshRenderer)
        {
            // nBone(UINT)					// 뼈의 개수
            binaryWriter.Write(skinnedMeshRenderer.bones.Length);
            // boneNames(string * nBone)		// 뼈의 이름들
            foreach (Transform boneTransform in skinnedMeshRenderer.bones)
            {
                BinaryWriteString(boneTransform.name, binaryWriter);
            }

            // 애니메이션 정보를 읽어서 출력한다.
            ObjectInfo objectInfo = skinnedMeshRenderer.GetComponent<ObjectInfo>();
            if (objectInfo)
            {
                // nAnimationSet (UINT)		// 애니메이션 클립의 개수
                binaryWriter.Write((uint)objectInfo.animationClips.Length);
                foreach (AnimationClip clip in objectInfo.animationClips)
                {
                    int nFramesPerSec = (int)clip.frameRate;
                    int nKeyFrames = Mathf.CeilToInt(clip.length * nFramesPerSec);

                    // animationSetName (string)	// 애니메이션 클립의 이름
                    BinaryWriteString(clip.name, binaryWriter);
                    // animationSetRuntime (float)	// 애니메이션 클립의 런타임
                    binaryWriter.Write(clip.length);
                    // animationNKeyFrame (UINT)	// 키프레임의 수
                    binaryWriter.Write((uint)nKeyFrames);

                    // 각 뼈의 애니메이션을 저장한다.
                    float fFrameRate = (1.0f / nFramesPerSec), fKeyFrameTime = 0.0f;
                    SRT[,] animationSet = new SRT[skinnedMeshRenderer.bones.Length, nKeyFrames];
                    for (int k = 0; k < nKeyFrames; k++)
                    {
                        clip.SampleAnimation(skinnedMeshRenderer.gameObject, fKeyFrameTime);
                        for (int boneIndex = 0; boneIndex < skinnedMeshRenderer.bones.Length; boneIndex++)
                        {
                            animationSet[boneIndex, k].scale = skinnedMeshRenderer.bones[boneIndex].localScale;
                            animationSet[boneIndex, k].rotation = skinnedMeshRenderer.bones[boneIndex].localRotation;
                            animationSet[boneIndex, k].position = skinnedMeshRenderer.bones[boneIndex].position;
                        }
                        fKeyFrameTime += fFrameRate;
                    }

                    //  ( * nBone)          // 각 뼈에 대해서
                    for (int boneIndex = 0; boneIndex < skinnedMeshRenderer.bones.Length; boneIndex++)
                    {
                        //  ( * animationNKeyFrame)     // 키프레임 수 만큼
                        for (int k = 0; k < nKeyFrames; ++k)
                        {
                            // scale (float * 3) + rotation (float * 4) + position (float * 3)      //SRT
                            BinaryWriteVector3(animationSet[boneIndex, k].scale, binaryWriter);
                            BinaryWriteQuat(animationSet[boneIndex, k].rotation, binaryWriter);
                            BinaryWriteVector3(animationSet[boneIndex, k].position, binaryWriter);
                        }
                    }

                }
            }
            else
            {
                binaryWriter.Write((uint)0);
                Debug.Log("애니메이션 클립 정보를 읽기 위한 ObjectInfo 스트립트가 존재하지 안습니다.");
            }

        }

        string objectName = objectTransform.name;
        Debug.Log(objectName.Length);
        Debug.Log(skinnedMeshRenderer);
        Debug.Log(objectName);  

        // nameSize (UINT) / name(string)
        BinaryWriteString(objectName, binaryWriter);

        //localPosition(float3)
        BinaryWriteVector3(curObjectTransform.localPosition, binaryWriter);

        //localScale(float3)
        BinaryWriteVector3(curObjectTransform.localScale, binaryWriter);

        //localRotation(float4)
        BinaryWriteQuat(curObjectTransform.localRotation, binaryWriter);

        //Debug.Log(curObjectTransform.localPosition);
        //Debug.Log(curObjectTransform.localScale);
        //Debug.Log(curObjectTransform.localRotation);
        MeshFilter meshFilter = curObjectTransform.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = curObjectTransform.GetComponent<MeshRenderer>();
        

        if (meshFilter && meshRenderer)    // 메쉬가 있는 경우
        {
            // haveMesh (bool)
            binaryWriter.Write(1);


            CreateMeshBinaryFile(meshFilter.sharedMesh, ref modelBound, curObjectTransform.position, binaryWriter);

            // material 리스트 정보
            Material[] materialList = meshRenderer.sharedMaterials;

            // materialSize(UINT)
            binaryWriter.Write((uint)materialList.Length);
            //Debug.Log((uint)materialList.Length);

            foreach (Material mat in materialList)
            {
                BinaryWriteMaterial(mat, binaryWriter);
            }

        }
        else if (skinnedMeshRenderer)
        {
            binaryWriter.Write(2);
            CreateSkinnedMeshStreamFile(skinnedMeshRenderer, ref modelBound, curObjectTransform.position, binaryWriter);

            // material 리스트 정보
            Material[] materialList = skinnedMeshRenderer.sharedMaterials;

            // materialSize(UINT)
            binaryWriter.Write((uint)materialList.Length);
            //Debug.Log((uint)materialList.Length);

            foreach (Material mat in materialList)
            {
                BinaryWriteMaterial(mat, binaryWriter);
            }
        }
        else  // 메쉬가 없는 경우
        {
            binaryWriter.Write(0);
        }

        // nChildren(UINT)
        binaryWriter.Write(curObjectTransform.childCount);
        //.Log(curObjectTransform.childCount + "개이다.");
        for (int i = 0; i < curObjectTransform.childCount; i++)  // 자식들을 똑같은 포멧으로 저장
        {
            // vec = worldPosition
            CreateObjectBinaryFile(curObjectTransform.GetChild(i), binaryWriter, ref modelBound);
             
        }

    }

    void MakeModel(Transform _gameObject)
    {
        string fileName = _gameObject.name;
        // 모델을 전부 덮는 커버 OOBB
        Bounds modelBound = new Bounds();
        
        modelBound.size = Vector3.zero;
        
        DirectoryInfo directoryInfo = new DirectoryInfo("ModelBinaryFile/");
        if (directoryInfo.Exists == false)
        {
            directoryInfo.Create();
        }
        BinaryWriter binaryWriter = new BinaryWriter(File.Open("ModelBinaryFile/" + fileName, FileMode.Create));

        CreateObjectBinaryFile(_gameObject, binaryWriter, ref modelBound);
        BinaryWriteVector3(modelBound.center, binaryWriter);
        BinaryWriteVector3(modelBound.extents, binaryWriter);

        binaryWriter.Flush();
        binaryWriter.Close();
    }
    // Start is called before the first frame update
    void Start()
    {
        for (int i = 0; i < transform.childCount; i++)  // 자식들을 똑같은 포멧으로 저장
        {
            MakeModel(transform.GetChild(i));
        }
    }

}
