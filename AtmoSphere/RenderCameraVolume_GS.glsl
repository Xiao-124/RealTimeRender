#version 430 core


layout (triangles) in;     //输入三角形，2次调用
layout (triangle_strip, max_vertices = 3) out;  //输出三角形

in vec2 gTexCoord[];    //从Vertex传过来的纹理坐标
flat in int layer[];

out vec2 fTexCoord;     //传到Fragment去的纹理坐标
//out int gl_Layer;   //层数的标记

void main()
{
    for(int k=0; k<gl_in.length(); k++)   //针对三角形每个顶点
    {
        //gl_Layer = gl_InvocationID;    //用调用编号标记层号
        //gl_Layer = layer[k];
        fTexCoord = gTexCoord[k];    //纹理坐标传递
        gl_Position = gl_in[k].gl_Position;    //顶点坐标传递
        EmitVertex();    //开始传递顶点信息
    }
    EndPrimitive();    //结束
}