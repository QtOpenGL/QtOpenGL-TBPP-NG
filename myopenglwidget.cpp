// 星图控件
// Star Chart Control

#include "myopenglwidget.h"
#include "color.cpp"

const float PLANET_Z = 2.0;                // 星球的z坐标  // The z-coordinate of the planet
const float PLANET_DRAW_POS_RATIO = 0.05;  // 星球坐标与场景坐标的比例 // The ratio of the planet's coordinates to the scene's coordinates

const int MAX_BLUR_ITER = 4;

const float MOVE_SPD_KEY = 0.01;
const float MOVE_DUMP = 0.8;

const float EDGE_X = 2.0;
const float EDGE_Y = 1.5;
const float EDGE_Z_NEAR = PLANET_Z - 0.1;
const float EDGE_Z_FAR = -3.0;
const float EDGE_ELAS = -0.5;

const float MOVE_SPD_MOUSE_Z = 0.002;
const float MOVE_SPD_MOUSE_EDGE = 0.01;
const int EDGE_MOUSE_RANGE = 200;
const float MOVE_SPD_WHEEL = 0.1;

// 初始暂停绘制，后端初始化完毕后才能开始绘制
// initial pause drawing, the back-end initialization is complete before drawing
MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      paused(true),
      showFriendship(false),
      showParentCivil(false),
      showFleet(false),
      xPos(0.0),
      yPos(0.0),
      zPos(0.0),
      xSpd(0.0),
      ySpd(0.0),
      zSpd(0.0),
      mouseInLeftEdge(false),
      mouseInRightEdge(false),
      mouseInTopEdge(false),
      mouseInBottomEdge(false),
      selectedPlanetId(-1),
      frameCount(0),
      lastFpsTime(0),
      fps(0.0)
{
    // lastMousePoint用默认初始化
    // lastMousePoint is initialized by default
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

MyOpenGLWidget::~MyOpenGLWidget()
{
    // 释放绘图用的缓冲区
    // Release the drawing with the buffer
    vertexArray.destroy();
    scrVertexArray.destroy();
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteFramebuffers(1, &frameBuffer);
    glDeleteTextures(2, colorBuffers);
    glDeleteRenderbuffers(1, &depthBuffer);
    glDeleteFramebuffers(2, pingpongFrameBuffers);
    glDeleteTextures(2, pingpongColorBuffers);
    glDeleteBuffers(1, &scrVertexBuffer);
}

void MyOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    int retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);
    glEnable(GL_DEPTH_TEST);

    // 初始化着色器
    // Initialize the shader
    shader.addShaderFromSourceCode(QOpenGLShader::Vertex,
#include "shaders/planet.vert"
                                   );
    shader.addShaderFromSourceCode(QOpenGLShader::Fragment,
#include "shaders/planet.frag"
                                   );
    shader.link();

    shaderLine.addShaderFromSourceCode(QOpenGLShader::Vertex,
#include "shaders/line.vert"
                                       );
    shaderLine.addShaderFromSourceCode(QOpenGLShader::Fragment,
#include "shaders/line.frag"
                                       );
    shaderLine.link();

    shaderBlur.addShaderFromSourceCode(QOpenGLShader::Vertex,
#include "shaders/blur.vert"
                                       );
    shaderBlur.addShaderFromSourceCode(QOpenGLShader::Fragment,
#include "shaders/blur.frag"
                                       );
    shaderBlur.link();

    shaderPost.addShaderFromSourceCode(QOpenGLShader::Vertex,
#include "shaders/post.vert"
                                       );
    shaderPost.addShaderFromSourceCode(QOpenGLShader::Fragment,
#include "shaders/post.frag"
                                       );
    shaderPost.link();

    shaderPost.bind();
    shaderPost.setUniformValue("image", 0);
    shaderPost.setUniformValue("blur", 1);
    shaderPost.release();

    // 初始化缓冲区
    // Initialize the buffer
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    glGenTextures(2, colorBuffers);
    for (int i = 0; i < 2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width(), height(), 0, GL_RGB,
                     GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 边缘设为剪切
        // The edge is set to Cut
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                               GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width(),
                          height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, depthBuffer);

    GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(2, pingpongFrameBuffers);
    glGenTextures(2, pingpongColorBuffers);
    for (int i = 0; i < 2; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFrameBuffers[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width(), height(), 0, GL_RGB,
                     GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, pingpongColorBuffers[i], 0);
    }

    // 初始化星球的顶点数组，未写入数据
    // Initializes the vertex array of the planet without writing data
    vertexArray.create();
    glGenBuffers(1, &vertexBuffer);

    // 初始化屏幕的顶点数组
    // Initializes the vertex array of the screen
    const GLfloat quadVertices[] = {
        // 坐标              // 材质坐标
        // coordinates      // material coordinates
        -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  // 0
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // 1
        1.0f,  1.0f,  0.0f, 1.0f, 1.0f,  // 2
        1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,  // 3
    };

    scrVertexArray.create();
    scrVertexArray.bind();
    glGenBuffers(1, &scrVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, scrVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          (GLvoid *)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    scrVertexArray.release();

    // 初始化投影矩阵
    // Initialize the projection matrix
    projMat.perspective(60.0, float(width()) / float(height()), 0.1, 100.0);

}

void MyOpenGLWidget::paintGL()
{
    if (paused) return;
    backend->lock();

    // 在frameBuffer中绘制星球
    // Plot the planet in the frameBuffer
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.bind();
    vertexArray.bind();

    // 确定星球颜色时以科技最高的星球为参考
    // Determine the color of the planet when the planet with the highest technology as a reference
    double maxTech = 0.0;
    for (int i = 0; i < MAX_PLANET; ++i)
        maxTech = max(maxTech, civils[planets[i].civilId].tech);

    for (int i = 0; i < MAX_PLANET; ++i)
    {
        Planet &p = planets[i];
        // 将星球坐标转换为场景坐标
        // Converts the planet coordinates to the scene coordinates
        float drawX = planetToDrawPos(p.x);
        float drawY = planetToDrawPos(p.y);
        float r = p.radius;

        // 根据星球科技确定饱和度与亮度
        // According to the planet to determine the saturation and brightness
        Civil &c = civils[p.civilId];
        float tTech = c.tech / maxTech;

        float tHue;
        if (showFriendship && selectedPlanetId >= 0 &&
            p.planetId != selectedPlanetId)
        {
            float tFriendship = Civil::friendship[selectedPlanetId][p.planetId];
            if (tFriendship > 0.0)
                tHue = 0.3 + 0.3 * (1.0 - exp(-tFriendship));
            else
                tHue = 0.3 * exp(tFriendship);
        }
        else
        {
            // 根据星球发展、攻击、合作概率之比确定色相
            // 攻击偏红色，发展偏绿色，合作偏蓝色
            // According to the planet development, attack, the probability of cooperation than to determine the hue
            // Attack partial red, the development of partial green, cooperation blue
            float tRed = abs(c.rateAtk);
            float tGreen = abs(c.rateDev);
            float tBlue = abs(c.rateCoop);
            if (tRed > tBlue)
                tHue = 0.3 * tGreen / (tRed + tGreen);
            else
                tHue = 0.3 + 0.3 * tBlue / (tGreen + tBlue);
        }

        RgbColor tColor = hsvToRgb(HsvColor(round(255.0 * tHue),
                                            128 + round(127.0 * (1.0 - tTech)),
                                            64 + round(191.0 * tTech)));
        drawCircle(drawX, drawY, r, float(tColor.r) / 255.0,
                   float(tColor.g) / 255.0, float(tColor.b) / 255.0);
    }

    vertexArray.release();
    shader.release();

    shaderLine.bind();
    vertexArray.bind();

    if (selectedPlanetId >= 0)
    {
        // 绘制选中星球的圆圈，无发光效果
        // Draw selected circle of the planet, no light effect
        Planet &p = planets[selectedPlanetId];
        // 将星球坐标转换为场景坐标
        // Converts the planet coordinates to the scene coordinates
        float selfDrawX = planetToDrawPos(p.x);
        float selfDrawY = planetToDrawPos(p.y);
        float r = p.radius + 0.005;
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawCircle(selfDrawX, selfDrawY, r, 1.0, 1.0, 1.0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // 绘制选中星球到母星球的线段
        // Plot the line from the selected planet to the parent planet
        const int MAX_VERTICES = 60;
        if (showParentCivil)
        {
            vector<GLfloat> vertices;
            //            vertices.reserve(MAX_PLANET * 6);
            int nowCivilId = p.civilId;
            while (true)
            {
                Civil &c = civils[nowCivilId];
                if (c.parentCivilId == -1 || vertices.size() >= MAX_VERTICES)
                    break;
                Planet &p2 = planets[c.planetId];
                vertices.push_back(planetToDrawPos(p2.x));
                vertices.push_back(planetToDrawPos(p2.y));
                vertices.push_back(0.0);

                Planet &p3 = planets[civils[c.parentCivilId].planetId];
                vertices.push_back(planetToDrawPos(p3.x));
                vertices.push_back(planetToDrawPos(p3.y));
                vertices.push_back(0.0);

                nowCivilId = c.parentCivilId;
            }
            if (vertices.size() >= 6) drawLine(vertices, 1.0, 1.0, 1.0);
        }

        // 绘制选中星球附近的舰队
        // Draw the selected fleet near the planet
        if (showFleet)
        {
            vector<GLfloat> verticesAtk, verticesCoop;
            for (auto i = fleets.begin(); i != fleets.end(); ++i)
            {
                if ((civils[i->fromCivilId].planetId == p.planetId ||
                     space.isNear[civils[i->fromCivilId].planetId]
                                 [p.planetId]) &&
                    (i->targetPlanetId == p.planetId ||
                     space.isNear[i->targetPlanetId][p.planetId]))
                {
                    if (i->actType == ACT_ATK)
                    {
                        Planet &pFrom =
                            planets[civils[i->fromCivilId].planetId];
                        float fromDrawX = planetToDrawPos(pFrom.x);
                        float fromDrawY = planetToDrawPos(pFrom.y);
                        verticesAtk.push_back(fromDrawX);
                        verticesAtk.push_back(fromDrawY);
                        verticesAtk.push_back(0.0);

                        Planet &pTarget = planets[i->targetPlanetId];
                        float targetDrawX = planetToDrawPos(pTarget.x);
                        float targetDrawY = planetToDrawPos(pTarget.y);
                        float progress = i->remainDis / i->initDis;
                        verticesAtk.push_back(fromDrawX * progress +
                                              targetDrawX * (1.0 - progress));
                        verticesAtk.push_back(fromDrawY * progress +
                                              targetDrawY * (1.0 - progress));
                        verticesAtk.push_back(0.0);
                    }
                    else  // i->actType == ACT_Coop
                    {
                        Planet &pFrom =
                            planets[civils[i->fromCivilId].planetId];
                        float fromDrawX = planetToDrawPos(pFrom.x);
                        float fromDrawY = planetToDrawPos(pFrom.y);
                        verticesCoop.push_back(fromDrawX);
                        verticesCoop.push_back(fromDrawY);
                        verticesCoop.push_back(0.0);

                        Planet &pTarget = planets[i->targetPlanetId];
                        float targetDrawX = planetToDrawPos(pTarget.x);
                        float targetDrawY = planetToDrawPos(pTarget.y);
                        float progress = i->remainDis / i->initDis;
                        verticesCoop.push_back(fromDrawX * progress +
                                               targetDrawX * (1.0 - progress));
                        verticesCoop.push_back(fromDrawY * progress +
                                               targetDrawY * (1.0 - progress));
                        verticesCoop.push_back(0.0);
                    }
                }
            }
            if (verticesAtk.size() >= 6) drawLine(verticesAtk, 1.0, 0.0, 0.0);
            if (verticesCoop.size() >= 6) drawLine(verticesCoop, 0.0, 0.5, 1.0);
        }
    }

    vertexArray.release();
    shaderLine.release();

    backend->unlock();

    // 在pingpongFrameBuffers中绘制发光效果
    // Draw the glow in pingpongFrameBuffers
    shaderBlur.bind();
    bool horizontal = true;
    for (int i = 0; i < MAX_BLUR_ITER; ++i)
    {
        shaderBlur.setUniformValue("horizontal", horizontal);
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFrameBuffers[horizontal]);
        glActiveTexture(GL_TEXTURE0);
        if (i == 0)
            glBindTexture(GL_TEXTURE_2D, colorBuffers[1]);
        else
            glBindTexture(GL_TEXTURE_2D, pingpongColorBuffers[!horizontal]);
        // 绘制屏幕
        // Draw the screen
        scrVertexArray.bind();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        scrVertexArray.release();
        horizontal = !horizontal;
    }
    shaderBlur.release();

    // 将colorBuffers[0]中的暗区与pingpongColorBuffers中的亮区合成
    // Combine the dark area in colorBuffers [0] with the bright area in the pingpongColorBuffers
    shaderPost.bind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongColorBuffers[!horizontal]);
    // 绘制屏幕
    // Draw the screen
    scrVertexArray.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    scrVertexArray.release();
    shaderPost.release();
}

// 留空
// Leave blank
void MyOpenGLWidget::resizeGL()
{
}

// x，y，r为场景坐标
// x, y, r is the scene coordinates
void MyOpenGLWidget::drawCircle(float x, float y, float r, float colorR,
                                float colorG, float colorB)
{
    // 将场景坐标转换为屏幕坐标
    // 半径大的星球略微往下移动，防止挡住半径小的星球
    // Converts scene coordinates to screen coordinates
    // The planet with a large radius moves slightly downwards to prevent it from blocking the planet with a small radius
    QMatrix4x4 matrix = projMat;
    matrix.translate(xPos + x, yPos + y, zPos - PLANET_Z - r);

    // 在视线外则不绘制
    // Do not draw outside the line of sight
    QVector4D testVec = matrix * QVector4D(0.0, 0.0, 0.0, 1.0);
    if (abs(testVec.x()) > testVec.w() * 1.2) return;
    if (abs(testVec.y()) > testVec.w() * 1.2) return;
    // Qt的语义有问题，setUniformValue对任何shader都有效，而不是当前的shader
    // Qt has a semantic problem, setUniformValue is valid for any shader, not the current shader
    shader.setUniformValue("matrix", matrix);

    // 防止颜色超出范围
    // Prevents color out of range
#define chop(a) a = min(max(a, 0.0f), 1.0f)
    chop(colorR);
    chop(colorG);
    chop(colorB);
#undef chop
    shader.setUniformValue("color", colorR, colorG, colorB);

    // 根据半径和缩放调整画圆的线段数
    // Adjust the number of lines in the circle according to the radius and the scale
    const int MIN_CIRCLE_SEG = 6;
    const int MAX_CIRCLE_SEG = 50;

    int segCount = floor(1000.0 * r / abs(PLANET_Z - zPos));
    if (segCount < MIN_CIRCLE_SEG) segCount = MIN_CIRCLE_SEG;
    if (segCount > MAX_CIRCLE_SEG) segCount = MAX_CIRCLE_SEG;

    // 画圆用到的临时变量
    // Draw a temporary variable for the circle
    float th = 2.0 * M_PI / float(segCount);
    float c = cosf(th);
    float s = sinf(th);
    float px = r;
    float py = 0;

    // 构造顶点数组
    // Construct an array of vertices
    GLfloat vertices[MAX_CIRCLE_SEG * 3];
    for (int i = 0; i < segCount; ++i)
    {
        vertices[i * 3] = px;
        vertices[i * 3 + 1] = py;
        vertices[i * 3 + 2] = 0.0;
        float t = px;
        px = c * px - s * py;
        py = s * t + c * py;
    }

    // 写入顶点数组
    // Writes an array of vertices
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, segCount * 3 * sizeof(GLfloat), &vertices,
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
                          (GLvoid *)(0));
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // 绘制
    // Draw
    glDrawArrays(GL_POLYGON, 0, segCount);
}

void MyOpenGLWidget::drawLine(vector<GLfloat> &vertices, float colorR,
                              float colorG, float colorB)
{
    // 将场景坐标转换为屏幕坐标
    // 半径大的星球略微往下移动，防止挡住半径小的星球    
    // Converts scene coordinates to screen coordinates
    // The planet with a large radius moves slightly downwards to prevent it from blocking the planet with a small radius
    QMatrix4x4 matrix = projMat;
    matrix.translate(xPos, yPos, zPos - PLANET_Z);
    shader.setUniformValue("matrix", matrix);

// 防止颜色超出范围
// Prevents color out of range
#define chop(a) a = min(max(a, 0.0f), 1.0f)
    chop(colorR);
    chop(colorG);
    chop(colorB);
#undef chop
    shader.setUniformValue("color", colorR, colorG, colorB);

    // 写入顶点数组
    // Writes an array of vertices
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat),
                 &vertices[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
                          (GLvoid *)(0));
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // 绘制
    // Draw
    glDrawArrays(GL_LINES, 0, vertices.size());
}

void MyOpenGLWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        // z坐标离星球越近，移动速度越慢
        // z coordinates closer to the planet, the slower the movement
        case Qt::Key_L:
            xSpd -= MOVE_SPD_KEY * (PLANET_Z - zPos);
            break;
        case Qt::Key_J:
            xSpd += MOVE_SPD_KEY * (PLANET_Z - zPos);
            break;
        case Qt::Key_I:
            ySpd -= MOVE_SPD_KEY * (PLANET_Z - zPos);
            break;
        case Qt::Key_K:
            ySpd += MOVE_SPD_KEY * (PLANET_Z - zPos);
            break;
        case Qt::Key_U:
            zSpd += MOVE_SPD_KEY * (PLANET_Z - zPos);
            break;
        case Qt::Key_O:
            zSpd -= MOVE_SPD_KEY * (PLANET_Z - zPos);
            break;
    }
}

void MyOpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::RightButton)
    {
        // 右键拖动时禁用鼠标靠近屏幕边缘的效果        
        // right mouse button drag to disable the effect of the mouse near the edge of the screen
        mouseInLeftEdge = mouseInRightEdge = mouseInTopEdge =
            mouseInBottomEdge = false;

        QPoint delta = event->pos() - lastMousePoint;
        // 鼠标位置与上次记录时较近，才是连续移动        
        // Mouse position and the last record is closer, is the continuous movement
        if (delta.manhattanLength() < 10)
        {
            zSpd -= MOVE_SPD_MOUSE_Z * delta.y() * (PLANET_Z - zPos);
        }
        lastMousePoint = event->pos();
    }
    else
    {
        if (event->pos().x() > width() - EDGE_MOUSE_RANGE)
            mouseInLeftEdge = true;
        else
            mouseInLeftEdge = false;
        if (event->pos().x() < EDGE_MOUSE_RANGE)
            mouseInRightEdge = true;
        else
            mouseInRightEdge = false;
        if (event->pos().y() > height() - EDGE_MOUSE_RANGE)
            mouseInTopEdge = true;
        else
            mouseInTopEdge = false;
        if (event->pos().y() < EDGE_MOUSE_RANGE)
            mouseInBottomEdge = true;
        else
            mouseInBottomEdge = false;
    }
}

void MyOpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        // 将场景坐标转换为屏幕坐标，获得z，w坐标        
        // Convert the scene coordinates to the screen coordinates to get the z, w coordinates
        QMatrix4x4 matrix = projMat;
        matrix.translate(xPos, yPos, zPos - PLANET_Z);
        QVector4D testVec = matrix * QVector4D(0.0, 0.0, 0.0, 1.0);
        // 将鼠标坐标转换为屏幕坐标        
        // Converts the mouse coordinates to the screen coordinates
        float scrX = (float(event->pos().x()) / float(width()) * 2.0 - 1.0);
        float scrY = (1.0 - float(event->pos().y()) / float(height()) * 2.0);
        // 将屏幕坐标转换为场景坐标
        // Converts screen coordinates to scene coordinates
        matrix.setToIdentity();
        matrix.translate(-xPos, -yPos, -zPos + PLANET_Z);
        matrix *= projMat.inverted();
        testVec = matrix * QVector4D(scrX * testVec.w(), scrY * testVec.w(),
                                     testVec.z(), testVec.w());

        // 将场景坐标转换为星球坐标        
        // Converts scene coordinates to planetary coordinates
        float planetX = drawToPlanetPos(testVec.x());
        float planetY = drawToPlanetPos(testVec.y());

        // 遍历所有星球，选择半径最小的星球，防止被大的星球挡住
        // 速度较慢，无法用于鼠标移动时判定
        // TODO：减少遍历数
        // Traverse all planets, select the smallest radius of the planet, to prevent being blocked by a large planet
        // Slower, can not be used to determine the mouse movement
        // TODO: Reduce the traversal number
        int nowPlanetId = -1;
        float nowR = numeric_limits<float>::infinity();
        for (int i = 0; i < MAX_PLANET; ++i)
        {
            // 星球半径太小时给一个较大的选择半径
            // Planet radius is too small to give a larger selection radius
            float selectR =
                max(planets[i].radius, 0.01f) / PLANET_DRAW_POS_RATIO;
            if (sqr(planetX - planets[i].x) + sqr(planetY - planets[i].y) <
                sqr(selectR))
            {
                if (selectR < nowR)
                {
                    nowPlanetId = planets[i].planetId;
                    nowR = selectR;
                }
            }
        }
        selectedPlanetId = nowPlanetId;
    }
}

void MyOpenGLWidget::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0)
        zPos += MOVE_SPD_WHEEL * (PLANET_Z - zPos);
    else if (event->delta() < 0)
        zPos -= MOVE_SPD_WHEEL * (PLANET_Z - zPos);
}

float MyOpenGLWidget::planetToDrawPos(float x)
{
    return (x - CENTER_POS) * PLANET_DRAW_POS_RATIO;
}

float MyOpenGLWidget::drawToPlanetPos(float x)
{
    return x / PLANET_DRAW_POS_RATIO + CENTER_POS;
}

void MyOpenGLWidget::animate()
{
    if (paused) return;

    if (mouseInLeftEdge)
        xSpd += -MOVE_SPD_MOUSE_EDGE * (PLANET_Z - zPos);
    else if (mouseInRightEdge)
        xSpd += MOVE_SPD_MOUSE_EDGE * (PLANET_Z - zPos);
    if (mouseInTopEdge)
        ySpd += MOVE_SPD_MOUSE_EDGE * (PLANET_Z - zPos);
    else if (mouseInBottomEdge)
        ySpd += -MOVE_SPD_MOUSE_EDGE * (PLANET_Z - zPos);

    xSpd *= MOVE_DUMP;
    ySpd *= MOVE_DUMP;
    zSpd *= MOVE_DUMP;

    xPos += xSpd;
    yPos += ySpd;
    zPos += zSpd;

    if (xPos > EDGE_X)
    {
        xPos = EDGE_X;
        xSpd *= EDGE_ELAS;
    }
    if (xPos < -EDGE_X)
    {
        xPos = -EDGE_X;
        xSpd *= EDGE_ELAS;
    }
    if (yPos > EDGE_Y)
    {
        yPos = EDGE_Y;
        ySpd *= EDGE_ELAS;
    }
    if (yPos < -EDGE_Y)
    {
        yPos = -EDGE_Y;
        ySpd *= EDGE_ELAS;
    }
    if (zPos > EDGE_Z_NEAR)
    {
        zPos = EDGE_Z_NEAR;
        zSpd *= EDGE_ELAS;
    }
    if (zPos < EDGE_Z_FAR)
    {
        zPos = EDGE_Z_FAR;
        zSpd *= EDGE_ELAS;
    }

    update();

    ++frameCount;
    int nowFpsTime = gTime.elapsed();
    if (nowFpsTime - lastFpsTime > 100)
    {
        fps = float(frameCount) / float(nowFpsTime - lastFpsTime) * 1000.0;
        frameCount = 0;
        lastFpsTime = nowFpsTime;
    }
}
