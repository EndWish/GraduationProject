using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.IO;
using System.Text;
using UnityEditor;

public class MakeModelFileScript : MonoBehaviour
{
    [SerializeField]
    string fileName;
    void BinaryWriteString(string str, StreamWriter StreamWriter)
    {
        StreamWriter.WriteLine(str.Length);
        StreamWriter.WriteLine(str.ToCharArray());
    }
    void BinaryWriteVector3(Vector3 vector, StreamWriter StreamWriter)
    {
        StreamWriter.WriteLine(vector.x);
        StreamWriter.WriteLine(vector.y);
        StreamWriter.WriteLine(vector.z);
    }
    void BinaryWriteColor(Color c, StreamWriter StreamWriter)
    {
        StreamWriter.WriteLine(c.r);
        StreamWriter.WriteLine(c.g);
        StreamWriter.WriteLine(c.b);
        StreamWriter.WriteLine(c.a);
    }
    void BinaryWriteMatrix(Matrix4x4 matrix, StreamWriter StreamWriter)
    {
        StreamWriter.WriteLine(matrix.m00);
        StreamWriter.WriteLine(matrix.m10);
        StreamWriter.WriteLine(matrix.m20);
        StreamWriter.WriteLine(matrix.m30);
        StreamWriter.WriteLine(matrix.m01);
        StreamWriter.WriteLine(matrix.m11);
        StreamWriter.WriteLine(matrix.m21);
        StreamWriter.WriteLine(matrix.m31);
        StreamWriter.WriteLine(matrix.m02);
        StreamWriter.WriteLine(matrix.m12);
        StreamWriter.WriteLine(matrix.m22);
        StreamWriter.WriteLine(matrix.m32);
        StreamWriter.WriteLine(matrix.m03);
        StreamWriter.WriteLine(matrix.m13);
        StreamWriter.WriteLine(matrix.m23);
        StreamWriter.WriteLine(matrix.m33);
    }

    string CreateMaterialBinaryFile(Material material, string filePath)
    {
        StreamWriter StreamWriter = new StreamWriter(File.Open(filePath + material.name + "_material" + ".txt", FileMode.Create));

        // ambient(XMFLOAT4)
        Color ambient = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        BinaryWriteColor(ambient, StreamWriter);
        // diffuse(XMFLOAT4)
        if (material.HasProperty("_Color"))
        {
            Color diffuse = material.GetColor("_Color");
            BinaryWriteColor(diffuse, StreamWriter);
        }
        else
        {
            Color diffuse = new Color(0.0f, 0.0f, 0.0f, 1.0f);
            BinaryWriteColor(diffuse, StreamWriter);
        }
        // specular(XMFLOAT4)(specular.w = 반짝임계수)
        if (material.HasProperty("_SpecColor"))
        {
            Color specular = material.GetColor("_SpecColor");
            BinaryWriteColor(specular, StreamWriter);
        }
        else
        {
            Color specular = new Color(1.0f, 1.0f, 1.0f, 10.0f);
            BinaryWriteColor(specular, StreamWriter);
        }
        // emissive(XMFLOAT4)
        if (material.HasProperty("_EmissionColor"))
        {
            Color emission = material.GetColor("_EmissionColor");
            BinaryWriteColor(emission, StreamWriter);
        }
        else
        {
            Color emission = new Color(0.0f, 0.0f, 0.0f, 1.0f);
            BinaryWriteColor(emission, StreamWriter);
        }
        StreamWriter.Flush();
        StreamWriter.Close();
        Debug.Log(material.name + "_material");
        return material.name + "_material";
    }

    string CreateMeshBinaryFile(Mesh mesh, MeshRenderer meshRenderer, string filePath)
    {
        StreamWriter StreamWriter = new StreamWriter(File.Open(filePath + mesh.name + "_mesh" + ".txt", FileMode.Create));

        // nVertex(UINT)
        StreamWriter.WriteLine((uint)mesh.vertexCount);
        Debug.Log("정점수 : " + mesh.vertexCount);
        // nameSize (UINT) / name (string)
        BinaryWriteString(mesh.name + "_mesh", StreamWriter);
        // boundingBox (float * 6)
        BinaryWriteVector3(mesh.bounds.center, StreamWriter);
        BinaryWriteVector3(mesh.bounds.extents, StreamWriter);

        // positions (float * 3 * nVertex)
        foreach (Vector3 position in mesh.vertices)
            BinaryWriteVector3(position, StreamWriter);
        // normals (float * 3 * nVertex)
        foreach (Vector3 normal in mesh.normals)
            BinaryWriteVector3(normal, StreamWriter);

        // nSubMesh (UINT)
        Material[] materials = meshRenderer.materials;
        StreamWriter.WriteLine((uint)mesh.subMeshCount);
        for (int i = 0; i < mesh.subMeshCount; i++)
        {
            // nSubMeshIndex (UINT) / subMeshIndex (UINT * nSubMeshIndex)    ....( * nSubMesh )
            int[] subindicies = mesh.GetTriangles(i);
            StreamWriter.WriteLine(subindicies.Length);
            foreach (int index in subindicies)
                StreamWriter.WriteLine(index);
            // materialNameSize(UINT) / materialName(string)

            BinaryWriteString(CreateMaterialBinaryFile(materials[i], filePath), StreamWriter);
        }
        StreamWriter.Flush();
        StreamWriter.Close();
        Debug.Log(mesh.name + "_mesh");
        return mesh.name + "_mesh";
    }
    void CreateObjectBinaryFile(Transform curObjectTransform, StreamWriter StreamWriter, string filePath)
    {
        string objectName = curObjectTransform.name + "_gameobject";

        // nameSize (UINT) / name(string)
        BinaryWriteString(objectName, StreamWriter);

        // eachTransform (float4x4)
        
        Matrix4x4 eachTransform = Matrix4x4.identity;
        eachTransform.SetTRS(curObjectTransform.localPosition, curObjectTransform.localRotation, curObjectTransform.localScale);
        BinaryWriteMatrix(eachTransform, StreamWriter);

        // meshNameSize(UINT) / meshName(string)	=> 메쉬가 없을 경우 따로 처리하자
        MeshFilter meshFilter = curObjectTransform.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = curObjectTransform.GetComponent<MeshRenderer>();

        if (meshFilter && meshRenderer)    // 메쉬가 있는 경우
        {
            BinaryWriteString(CreateMeshBinaryFile(meshFilter.sharedMesh, meshRenderer, filePath), StreamWriter);
        }
        else  // 메쉬가 없는 경우
        {   
            StreamWriter.WriteLine(0);
        }

        // nChildren(UINT)
        StreamWriter.WriteLine(curObjectTransform.childCount);
        for(int i = 0; i < curObjectTransform.childCount; i++)  // 자식들을 똑같은 포멧으로 저장
        {
            CreateObjectBinaryFile(curObjectTransform.GetChild(i), StreamWriter, filePath);
        }
    }

    // Start is called before the first frame update
    void Start()
    {
        DirectoryInfo directoryInfo = new DirectoryInfo("ModelBinaryFile/" + fileName);
        if (directoryInfo.Exists == false)
        {
            directoryInfo.Create();
        }
        StreamWriter StreamWriter = new StreamWriter(File.Open("ModelBinaryFile/" + fileName + "/" + fileName + ".txt", FileMode.Create));
        CreateObjectBinaryFile(transform, StreamWriter, "ModelBinaryFile/" + fileName + "/");
        StreamWriter.Flush();
        StreamWriter.Close();

    }

}
