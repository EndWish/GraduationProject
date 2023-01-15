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
        streamWriter.Write(matrix.m00);
        streamWriter.Write(matrix.m10);
        streamWriter.Write(matrix.m20);
        streamWriter.Write(matrix.m30);
        streamWriter.Write(matrix.m01);
        streamWriter.Write(matrix.m11);
        streamWriter.Write(matrix.m21);
        streamWriter.Write(matrix.m31);
        streamWriter.Write(matrix.m02);
        streamWriter.Write(matrix.m12);
        streamWriter.Write(matrix.m22);
        streamWriter.Write(matrix.m32);
        streamWriter.Write(matrix.m03);
        streamWriter.Write(matrix.m13);
        streamWriter.Write(matrix.m23);
        streamWriter.Write(matrix.m33);
    }
    void StreamWriteMaterial(Material material, StreamWriter streamWriter)
    {

        StreamWriteStringWithoutLength("//////////// MaterialInfo //////////////", streamWriter);
        // materialNameSize(UINT) / materialName(string) -> 텍스처 이름

        StreamWriteString(material.name, streamWriter);

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
    }

    void CreateMeshStreamFile(Mesh mesh, MeshRenderer meshRenderer, ref Bounds modelBound, Vector3 vec, StreamWriter streamWriter)
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


        if (meshFilter && meshRenderer)    // 메쉬가 있는 경우
        {
            // haveMesh (bool)

            StreamWriteStringWithoutLength("haveMesh", streamWriter);
            StreamWriteInt(1, streamWriter);


            CreateMeshStreamFile(meshFilter.sharedMesh, meshRenderer, ref modelBound, curObjectTransform.position, streamWriter);

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
        else  // 메쉬가 없는 경우
        {
            StreamWriteInt(0, streamWriter);
        }

        // nChildren(UINT)
        streamWriter.Write(curObjectTransform.childCount);
        Debug.Log(curObjectTransform.childCount + "개이다.");
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
