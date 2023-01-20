using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.IO;
using System.Text;
using UnityEditor;
using Unity.VisualScripting;



public class MakeModelFileToText : MonoBehaviour
{
    struct SRT
    {
        public Vector3 scale;
        public Quaternion rotation;
        public Vector3 position;
    }

    void StreamWriteString(string str, StreamWriter streamWriter)
    {
        streamWriter.Write(str.Length + " " + str);
        streamWriter.WriteLine();
    }
    void StreamWriteInt(int i, StreamWriter streamWriter)
    {
        streamWriter.Write(i);
        streamWriter.WriteLine();
    }
    void StreamWriteStringWithoutLength(string str, StreamWriter streamWriter)
    {
        streamWriter.Write(str);
        streamWriter.WriteLine();

    }
    void StreamWriteVector3(Vector3 vector, StreamWriter streamWriter)
    {
        streamWriter.Write(vector.x + " " + vector.y + " " + vector.z);
        streamWriter.WriteLine();
    }
    void StreamWriteVector2(Vector2 vector, StreamWriter streamWriter)
    {
        streamWriter.Write(vector.x + " " + vector.y);
        streamWriter.WriteLine();
    }
    void StreamWriteQuat(Quaternion vector, StreamWriter streamWriter)
    {
        streamWriter.Write(vector.x + " " + vector.y + " " + vector.z + " " + vector.w);
        streamWriter.WriteLine();
    }

    void StreamWriteColor(Color c, StreamWriter streamWriter)
    {
        streamWriter.Write(c.r + " " + c.g + " " + c.b + " " + c.a);
        streamWriter.WriteLine();
    }
    void StreamWriteMatrix(Matrix4x4 matrix, StreamWriter streamWriter)
    {
        streamWriter.Write(matrix.m00 + " ");
        streamWriter.Write(matrix.m10 + " ");
        streamWriter.Write(matrix.m20 + " ");
        streamWriter.Write(matrix.m30 + " ");
        streamWriter.Write(matrix.m01 + " ");
        streamWriter.Write(matrix.m11 + " ");
        streamWriter.Write(matrix.m21 + " ");
        streamWriter.Write(matrix.m31 + " ");
        streamWriter.Write(matrix.m02 + " ");
        streamWriter.Write(matrix.m12 + " ");
        streamWriter.Write(matrix.m22 + " ");
        streamWriter.Write(matrix.m32 + " ");
        streamWriter.Write(matrix.m03 + " ");
        streamWriter.Write(matrix.m13 + " ");
        streamWriter.Write(matrix.m23 + " ");
        streamWriter.Write(matrix.m33 + " ");
        streamWriter.Write("\n");
    }
    void StreamWriteMaterial(Material material, StreamWriter streamWriter)
    {

        int materialType = 0;
        StreamWriteStringWithoutLength("//////////// MaterialInfo //////////////", streamWriter);
        // albedoNameSize(UINT) / albedoName(string) -> 알베도 텍스처 이름
        // bumpNameSize(UINT) / bumpName(string) -> 노말맵 텍스처 이름
        // 이후 추가
        string[] mapTypes = new string[2]
        {
            "_MainTex",
            "_BumpMap"
        };
        for (int i = 0; i < mapTypes.Length; ++i)
        {
            StreamWriteStringWithoutLength(mapTypes[i], streamWriter);
            
            Texture texture = material.GetTexture(mapTypes[i]);
            if (texture)
            {
                StreamWriteString(texture.name, streamWriter);
                materialType += 1 << i;
            }
            else
            {
                StreamWriteString("null", streamWriter);
            }

        }

        // ambient(XMFLOAT4)
        StreamWriteString("ambient", streamWriter);
        Color ambient = new Color(0.2f, 0.2f, 0.2f, 1.0f);

        StreamWriteColor(ambient, streamWriter);
        // diffuse(XMFLOAT4)

        StreamWriteString("diffuse", streamWriter);
        if (material.HasProperty("_Color"))
        {
            Color diffuse = material.GetColor("_Color");
            StreamWriteColor(diffuse, streamWriter);
        }
        else
        {
            Color diffuse = new Color(0.0f, 0.0f, 0.0f, 1.0f);
            StreamWriteColor(diffuse, streamWriter);
        }
        // specular(XMFLOAT4)(specular.w = 반짝임계수)

        StreamWriteString("specular", streamWriter);
        if (material.HasProperty("_SpecColor"))
        {

            Color specular = material.GetColor("_SpecColor");
            StreamWriteColor(specular, streamWriter);
        }
        else
        {
            Color specular = new Color(1.0f, 1.0f, 1.0f, 10.0f);
            StreamWriteColor(specular, streamWriter);
        }
        // emissive(XMFLOAT4)

        StreamWriteString("emissive", streamWriter);
        if (material.HasProperty("_EmissionColor"))
        {
            Color emission = material.GetColor("_EmissionColor");
            StreamWriteColor(emission, streamWriter);
        }
        else
        {
            Color emission = new Color(0.0f, 0.0f, 0.0f, 1.0f);
            StreamWriteColor(emission, streamWriter);
        }
        StreamWriteString("materialType", streamWriter);
        StreamWriteInt(materialType, streamWriter);

    }

    void CreateMeshStreamFile(Mesh mesh, ref Bounds modelBound, Vector3 vec, StreamWriter streamWriter)
    {

        StreamWriteStringWithoutLength("//////////// MeshInfo //////////////", streamWriter);
        // init modelbound

        Bounds worldBounds = new Bounds(mesh.bounds.center + vec, mesh.bounds.extents * 2);
        modelBound.Encapsulate(worldBounds);

        // nVertex(UINT)

        StreamWriteStringWithoutLength("nVertex", streamWriter);
        StreamWriteInt((int)mesh.vertexCount, streamWriter);

        // boundingBox (float * 6)

        StreamWriteStringWithoutLength("OOBB.Center", streamWriter);
        StreamWriteVector3(mesh.bounds.center, streamWriter);

        StreamWriteStringWithoutLength("OOBB.Extents", streamWriter);
        StreamWriteVector3(mesh.bounds.extents, streamWriter);

        // positions (float * 3 * nVertex)

        StreamWriteStringWithoutLength("positions", streamWriter);
        foreach (Vector3 position in mesh.vertices)
        {
            StreamWriteVector3(position, streamWriter);
        }
        // normals (float * 3 * nVertex)

        StreamWriteStringWithoutLength("normals", streamWriter);
        foreach (Vector3 normal in mesh.normals)
        {
            StreamWriteVector3(normal, streamWriter);
        }

        // numTexcoord (INT)

        StreamWriteStringWithoutLength("numTexcoord", streamWriter);
        StreamWriteInt((int)mesh.uv.Length, streamWriter);

        // texcoord (float * 2 * nVertex)

        StreamWriteStringWithoutLength("texcoords", streamWriter);
        foreach (Vector2 texcoord in mesh.uv)
        {
            Vector2 tmpuv = texcoord;
            tmpuv.y = 1 - tmpuv.y;
            StreamWriteVector2(tmpuv, streamWriter);
        }

        // nSubMesh (UINT)

        StreamWriteStringWithoutLength("nSubMesh", streamWriter);
        StreamWriteInt(mesh.subMeshCount, streamWriter);


        for (int i = 0; i < mesh.subMeshCount; i++)
        {
            // nSubMeshIndex (UINT) / subMeshIndex (UINT * nSubMeshIndex)    ....( * nSubMesh )
            int[] subindicies = mesh.GetTriangles(i);

            StreamWriteStringWithoutLength("nSubMeshIndex", streamWriter);
            StreamWriteInt(subindicies.Length, streamWriter);

            StreamWriteStringWithoutLength("subindicies", streamWriter);
            foreach (int index in subindicies)
            {
                StreamWriteInt(index, streamWriter);
            }
        }
    }

    void CreateSkinnedMeshStreamFile(SkinnedMeshRenderer skinnedMeshRenderer, ref Bounds modelBound, Vector3 vec, StreamWriter streamWriter)
    {

        StreamWriteStringWithoutLength("//////////// SkinnedMeshInfo //////////////", streamWriter);

        // 뼈하나당 영향의 주는 정점의 개수를 출력
        int nBonesPerVertex = 4;
        StreamWriteStringWithoutLength("뼈당 영향을 받는 정점의 개수" + nBonesPerVertex, streamWriter);

        // 뼈들의 이름을 출력
        StreamWriteStringWithoutLength("뼈의 개수" + skinnedMeshRenderer.bones.Length, streamWriter);
        foreach (Transform boneTransform in skinnedMeshRenderer.bones)
        {
            StreamWriteStringWithoutLength(boneTransform.gameObject.name, streamWriter);
        }

        Mesh mesh = skinnedMeshRenderer.sharedMesh;
        // 오프셋 행렬 출력
        //StreamWriteStringWithoutLength("오프셋 행렬의 개수" + skinnedMeshRenderer.sharedMesh.bindposes.Length, streamWriter);
        foreach (Matrix4x4 matrix in mesh.bindposes)  // 행렬의 개수는 뼈의 개수와 같다.
        {
            StreamWriteMatrix(matrix, streamWriter);
        }

        // 뼈의 가중치 정보 출력
        StreamWriteStringWithoutLength("뼈에 영향을 받는 정점의 개수 : " + mesh.boneWeights.Length, streamWriter);
        StreamWriteStringWithoutLength("두줄에 걸쳐 인덱스와 가중치를(각각4개씩 총 8개) x (정점의 수) 만큼 출력한다. : " + mesh.boneWeights.Length, streamWriter);
        foreach (BoneWeight boneWeight in mesh.boneWeights)
        {
            StreamWriteStringWithoutLength(boneWeight.boneIndex0 + " " + boneWeight.boneIndex1 + " " + boneWeight.boneIndex2 + " " + boneWeight.boneIndex3 + " ", streamWriter);
            StreamWriteStringWithoutLength(boneWeight.weight0 + " " + boneWeight.weight1 + " " + boneWeight.weight2 + " " + boneWeight.weight3 + " ", streamWriter);
        }

        // skinnnedMesh의 OOBB정보 출력
        StreamWriteStringWithoutLength("SkinnedMesh OOBB.Center", streamWriter);
        StreamWriteVector3(skinnedMeshRenderer.localBounds.center, streamWriter);
        StreamWriteStringWithoutLength("SkinnedMesh OOBB.Extents", streamWriter);
        StreamWriteVector3(skinnedMeshRenderer.localBounds.extents, streamWriter);

        // 애니메이션 정보를 읽어서 출력한다.
        ObjectInfo objectInfo = skinnedMeshRenderer.GetComponent<ObjectInfo>();
        if(objectInfo)
        {

            // 애니메이션 동작의 개수를 출력
            StreamWriteStringWithoutLength("애니메이션 동작의 개수 : " + objectInfo.animationClips.Length, streamWriter);
            foreach(AnimationClip clip in objectInfo.animationClips)
            {

                // 애니메이셔 동작의 행렬 출력
                int nFramesPerSec = (int)clip.frameRate;
                int nKeyFrames = Mathf.CeilToInt(clip.length * nFramesPerSec);
                StreamWriteStringWithoutLength("애니메이션 동작의 이름 : " + clip.name, streamWriter);
                StreamWriteStringWithoutLength("애니메이션 동작의 시간 : " + clip.length, streamWriter);
                StreamWriteStringWithoutLength("애니메이션 동작의 프레임수 : " + nKeyFrames, streamWriter);

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

                for (int boneIndex = 0; boneIndex < skinnedMeshRenderer.bones.Length; boneIndex++)
                {
                    StreamWriteStringWithoutLength("부위 : " + skinnedMeshRenderer.bones[boneIndex].name, streamWriter);
                    for (int k = 0; k < nKeyFrames; ++k)
                    {
                        StreamWriteStringWithoutLength(k + "번째 프레임 : ", streamWriter);
                        StreamWriteVector3(animationSet[boneIndex, k].scale, streamWriter);
                        StreamWriteQuat(animationSet[boneIndex, k].rotation, streamWriter);
                        StreamWriteVector3(animationSet[boneIndex, k].position, streamWriter);
                    }
                }

            }
        }
        else
        {
            Debug.Log("애니메이션 클립 정보를 읽기 위한 ObjectInfo 스트립트가 존재하지 안습니다.");
        }

        // 스킨드메쉬의 정보를 저장
        CreateMeshStreamFile(skinnedMeshRenderer.sharedMesh, ref modelBound, vec, streamWriter);
        
    }
    void CreateObjectStreamFile(Transform curObjectTransform, StreamWriter streamWriter, ref Bounds modelBound)
    {

        string objectName = curObjectTransform.name;

        // nameSize (UINT) / name(string)
        StreamWriteString(objectName, streamWriter);

        //localPosition(float3)

        StreamWriteStringWithoutLength("localPosition", streamWriter);
        StreamWriteVector3(curObjectTransform.localPosition, streamWriter);

        //localScale(float3)

        StreamWriteStringWithoutLength("localScale", streamWriter);
        StreamWriteVector3(curObjectTransform.localScale, streamWriter);

        //localRotation(float4)

        StreamWriteStringWithoutLength("localRotation", streamWriter);
        StreamWriteQuat(curObjectTransform.localRotation, streamWriter);

        MeshFilter meshFilter = curObjectTransform.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = curObjectTransform.GetComponent<MeshRenderer>();
        SkinnedMeshRenderer skinnedMeshRenderer = curObjectTransform.GetComponent<SkinnedMeshRenderer>();  

        if (meshFilter && meshRenderer)    // 메쉬가 있는 경우
        {
            // haveMesh (bool)

            StreamWriteStringWithoutLength("haveMesh", streamWriter);
            StreamWriteInt(1, streamWriter);


            CreateMeshStreamFile(meshFilter.sharedMesh, ref modelBound, curObjectTransform.position, streamWriter);

            // material 리스트 정보
            Material[] materialList = meshRenderer.sharedMaterials;

            // materialSize(UINT)

            StreamWriteStringWithoutLength("materialSize", streamWriter);
            StreamWriteInt(materialList.Length, streamWriter);

            foreach (Material mat in materialList)
            {
                StreamWriteMaterial(mat, streamWriter);
            }

        }
        else if (skinnedMeshRenderer)
        {
            StreamWriteStringWithoutLength("haveSkinnedMesh", streamWriter);


            CreateSkinnedMeshStreamFile(skinnedMeshRenderer, ref modelBound, curObjectTransform.position, streamWriter);
        }
        else  // 메쉬가 없는 경우
        {
            StreamWriteInt(0, streamWriter);
        }

        // nChildren(UINT)
        streamWriter.Write(curObjectTransform.childCount);
        for (int i = 0; i < curObjectTransform.childCount; i++)  // 자식들을 똑같은 포멧으로 저장
        {
            // vec = worldPosition
            CreateObjectStreamFile(curObjectTransform.GetChild(i), streamWriter, ref modelBound);

        }

    }

    void MakeModel(Transform _gameObject)
    {
        string fileName = _gameObject.name;
        // 모델을 전부 덮는 커버 OOBB
        Bounds modelBound = new Bounds();

        modelBound.size = Vector3.zero;

        DirectoryInfo directoryInfo = new DirectoryInfo("ModelStreamFile/");
        if (directoryInfo.Exists == false)
        {
            directoryInfo.Create();
        }
        StreamWriter streamWriter = new StreamWriter(File.Open("ModelStreamFile/" + fileName + ".txt", FileMode.Create));

        CreateObjectStreamFile(_gameObject.transform, streamWriter, ref modelBound);
        StreamWriteVector3(modelBound.center, streamWriter);
        StreamWriteVector3(modelBound.extents, streamWriter);

        streamWriter.Flush();
        streamWriter.Close();
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
