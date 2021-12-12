//
// Created by 12132 on 2021/12/11.
//

// openGL 生成shadow map 硬阴影 需要两趟
// 基本思路：
// render_pass 1 从 光源的角度记录各个物体最近的深度信息 生成深度贴图
// render_pass 2 从eye 角度利用第一次render_pass 生成的深度信息，比较当前的深度信息是否比已经记录的深度信息小
// 如果小那么该像素可见反正该像素不可见即在阴影中

// pcss  软硬影
int main()
{

}