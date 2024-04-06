#version 430 core


layout (triangles) in;     //���������Σ�2�ε���
layout (triangle_strip, max_vertices = 3) out;  //���������

in vec2 gTexCoord[];    //��Vertex����������������
flat in int layer[];

out vec2 fTexCoord;     //����Fragmentȥ����������
//out int gl_Layer;   //�����ı��

void main()
{
    for(int k=0; k<gl_in.length(); k++)   //���������ÿ������
    {
        //gl_Layer = gl_InvocationID;    //�õ��ñ�ű�ǲ��
        //gl_Layer = layer[k];
        fTexCoord = gTexCoord[k];    //�������괫��
        gl_Position = gl_in[k].gl_Position;    //�������괫��
        EmitVertex();    //��ʼ���ݶ�����Ϣ
    }
    EndPrimitive();    //����
}