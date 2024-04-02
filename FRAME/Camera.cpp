//--------------------------------------------------------------------------------------
// Copyright (c) Elay Pu. All rights reserved.
//--------------------------------------------------------------------------------------

#include "Camera.h"
#include <iostream>
#include "InputManager.h"
#include "App.h"
#include "common.h"
#include <glm/gtx/matrix_decompose.hpp>
#include<glm/gtc/quaternion.hpp>
#include <GL/glew.h>        
#include <math.h>                                    


CCamera::CCamera(glm::dvec3 vCameraPos, glm::dvec3 vCameraFront, glm::dvec3 vUpVector) :m_CameraPos(vCameraPos), m_CameraFront(vCameraFront), m_UpVector(vUpVector), m_PrevCameraPos(vCameraPos), m_PrevCameraFront(vCameraFront)
{
	m_CameraRight = glm::normalize(glm::cross(m_UpVector, -m_CameraFront));  //Fixed Me: - ?
	m_Pitch = asin(m_CameraFront.y);
	m_Yaw = asin(m_CameraFront.z / cos(m_Pitch));
	
}

CCamera::~CCamera()
{
}

//************************************************************************************
//Function:
GLvoid CCamera::init()
{
	std::shared_ptr<CInputManager> &pInputManager = CInputManager::getOrCreateInstance();
	pInputManager->registerKeyCallbackFunc(std::bind(&CCamera::__processMovement4KeyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	pInputManager->registerCursorCallbackFunc(std::bind(&CCamera::__processRotate4CursorCallback, this, std::placeholders::_1, std::placeholders::_2));
	pInputManager->registerScrollCallbackFunc(std::bind(&CCamera::__processScroll4ScrollCallback, this, std::placeholders::_1, std::placeholders::_2));
	pInputManager->registerMouseButtonCallbackFunc(std::bind(&CCamera::__processMouseButton4MouseButtonCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	
}


void _mapToSphere(const glm::vec2 &NewPt, glm::vec3 &NewVec, double NewWidth, double NewHeight)
{
	glm::vec2 TempPt;
	GLfloat length;

	double AdjustWidth = 1.0f / ((NewWidth - 1.0f) * 0.5f);
	double AdjustHeight = 1.0f / ((NewHeight - 1.0f) * 0.5f);

	//复制到临时变量
	TempPt = NewPt;

	//把长宽调整到[-1 ... 1]区间
	TempPt.x = (TempPt.x * AdjustWidth) - 1.0f;
	TempPt.y = 1.0f - (TempPt.y * AdjustHeight);


	//计算长度的平方
	length = (TempPt.x * TempPt.x) + (TempPt.y * TempPt.y);

	//如果点映射到球的外面
	if (length > 1.0f)
	{
		GLfloat norm;

		//缩放到球上
		norm = 1.0f / std::sqrt(length);

		//设置z坐标为0
		NewVec.x = TempPt.x * norm;
		NewVec.y = TempPt.y * norm;
		NewVec.z = 0.0f;
	}
	//如果在球内
	else
	{
		//利用半径的平方为1,求出z坐标
		NewVec.x = TempPt.x;
		NewVec.y = TempPt.y;
		NewVec.z = std::sqrt(1.0f - length);
	}
}


void drag(const glm::vec2 &NewPt, glm::vec3 &StVec, glm::quat &NewRot, double NewWidth, double NewHeight)
{
	//新的位置
	glm::vec3 EnVec;
	_mapToSphere(NewPt, EnVec, NewWidth, NewHeight);

	//计算旋转
	
	glm::vec3  Perp;
	
	//计算旋转轴
	Perp = glm::cross(StVec, EnVec);

	//如果不为0
	if (glm::length(Perp) > 1e-6)
	{
		//记录旋转轴
		NewRot.x = Perp.x;
		NewRot.y = Perp.y;
		NewRot.z = Perp.z;
		//在四元数中,w=cos(a/2)，a为旋转的角度
		NewRot.w = glm::dot(StVec, EnVec);;
		
	}
	//是0，说明没有旋转
	else
	{
			NewRot.x =
			NewRot.y =
			NewRot.z =
			NewRot.w = 0.0f;
	}
	
}


//************************************************************************************
//Function:
GLvoid CCamera::__processMovement4KeyCallback(GLint vKey, GLint vScancode, GLint vAction, GLint vMode)
{
}



//************************************************************************************
//Function:
GLvoid CCamera::__processRotate4CursorCallback(GLdouble vPosX, GLdouble vPosY)
{
	static bool isClick = false;
	static bool isDrag = false;
	static glm::vec3 startEnv;
	if (m_IsEnableCursor)
	{

		if (CInputManager::getOrCreateInstance()->getMouseButtonStatus()[0] )
		{
			std::array<double, 2> CursorOffset = CInputManager::getOrCreateInstance()->getCursorOffset();
			CursorOffset[0] *= m_Sensitivity;
			CursorOffset[1] *= m_Sensitivity;
			m_Yaw += glm::radians(CursorOffset[0]);
			m_Pitch += glm::radians(CursorOffset[1]);
			if (m_Pitch > glm::radians(89.0))
				m_Pitch = glm::radians(89.0);
			else if (m_Pitch < glm::radians(-89.0))
				m_Pitch = glm::radians(-89.0);
			m_CameraFront.x = cos(m_Pitch) * cos(m_Yaw);
			m_CameraFront.y = sin(m_Pitch);
			m_CameraFront.z = cos(m_Pitch) * sin(m_Yaw);
			m_CameraFront = glm::normalize(m_CameraFront);
			m_CameraRight = normalize(cross(m_UpVector, -m_CameraFront));


			//std::array<double, 2> CursorPos = CInputManager::getOrCreateInstance()->getCursorPos();
			//if (!isClick)
			//{
			//	isClick = true;
			//	_mapToSphere(glm::vec2(CursorPos[0], CursorPos[1]), startEnv, 1280, 760);
			//	isDrag = true;
			//	return;
			//}
			//
			//
			//if (isDrag)
			//{			
			//	glm::quat rotateQuat;
			//	drag(glm::vec2(CursorPos[0], CursorPos[1]), startEnv, rotateQuat, 1280, 760);
			//	_mapToSphere(glm::vec2(CursorPos[0], CursorPos[1]), startEnv, 1280, 760);
			//	modelMatrix = glm::mat4_cast(rotateQuat);
			//	//m_CameraPos = rotateMatrix * m_CameraPos;
			//}


		}
		else
		{
			//startEnv = glm::vec3(0, 0, 0);
			//isClick = false;
			//isDrag = false;
			//modelMatrix = glm::mat4(1);
		}
	}
	
}

//************************************************************************************
//Function:
GLvoid CCamera::__processScroll4ScrollCallback(GLdouble vOffsetX, GLdouble vOffsetY)
{
	if (m_Fov >= 1.0 && m_Fov <= 45.0)
		m_Fov -= vOffsetY;
	if (m_Fov < 1.0)
		m_Fov = 1.0;
	else if (m_Fov > 45.0)
		m_Fov = 45.0;
}

//************************************************************************************
//Function:
GLvoid CCamera::__processMouseButton4MouseButtonCallback(GLint vButton, GLint vAction, GLint vMods)
{
	if (vButton == GLFW_MOUSE_BUTTON_RIGHT && vAction == GLFW_PRESS)
	{
		std::cout << "CameraPos: " << m_CameraPos.x << ", " << m_CameraPos.y << ", " << m_CameraPos.z << std::endl;
		std::cout << "CameraFront: " << m_CameraFront.x << ", " << m_CameraFront.y << ", " << m_CameraFront.z << std::endl;
	}
}

//************************************************************************************
//Function:
GLdouble CCamera::getCameraFov() const 
{
	return m_Fov;
}

//************************************************************************************
//Function:
GLdouble CCamera::getCameraNear() const
{
	return m_Near;
}

//************************************************************************************
//Function:
GLdouble CCamera::getCameraFar() const
{
	return m_Far;
}

//************************************************************************************
//Function:
const glm::dvec3& CCamera::getCameraPos() const
{
	return m_CameraPos;
}

//************************************************************************************
//Function:
glm::dvec3 CCamera::getLookAtPos() const
{
	return m_CameraPos + m_CameraFront;
}

//************************************************************************************
//Function:
glm::dvec3 CCamera::getCameraFront() const
{
	return m_CameraFront;
}

//************************************************************************************
//Function:
const glm::dvec3& CCamera::getCameraUp() const
{
	return m_UpVector;
}

//************************************************************************************
//Function:
glm::mat4 CCamera::getViewMatrix() const 
{
	return glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_UpVector);
}

//************************************************************************************
//Function:
glm::mat4 CCamera::getProjectionMatrix() const
{
	//return glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 1000.0f);;
	return glm::perspective(glm::radians(m_Fov), (GLdouble)ElayGraphics::WINDOW_KEYWORD::WINDOW_WIDTH / (GLdouble)ElayGraphics::WINDOW_KEYWORD::WINDOW_HEIGHT, m_Near, m_Far);
}

//************************************************************************************
//Function:
void CCamera::update()
{
	m_PrevCameraPos = m_CameraPos;
	m_PrevCameraFront = m_CameraFront;

	std::shared_ptr<CInputManager> pInputManager = CInputManager::getOrCreateInstance();
	GLdouble MoveDistance = CApp::getOrCreateInstance()->getDeltaTime() * m_MoveSpeed;
	if (pInputManager->getKeyStatus()[GLFW_KEY_W])
	{		
		m_CameraPos += MoveDistance * glm::dvec3(m_CameraFront.x, 0, m_CameraFront.z);
	}
	if (pInputManager->getKeyStatus()[GLFW_KEY_S])
	{	
		m_CameraPos -= MoveDistance * glm::dvec3(m_CameraFront.x, 0, m_CameraFront.z);
	}
	if (pInputManager->getKeyStatus()[GLFW_KEY_D])
	{		
		m_CameraPos += MoveDistance * m_CameraRight;
	}
	if (pInputManager->getKeyStatus()[GLFW_KEY_A])
	{		
		m_CameraPos -= MoveDistance * m_CameraRight;
	}
	if (pInputManager->getKeyStatus()[GLFW_KEY_Q])
	{
		m_CameraPos -= MoveDistance * glm::dvec3(0.0, 1.0, 0.0);
	}
	if (pInputManager->getKeyStatus()[GLFW_KEY_E])
	{
		m_CameraPos += MoveDistance * glm::dvec3(0.0, 1.0, 0.0);
	}
	if (pInputManager->getKeyStatus()[GLFW_KEY_Z])
	{
		m_MoveSpeed *= 1.2f;
	}
	if (pInputManager->getKeyStatus()[GLFW_KEY_C])
	{
		m_MoveSpeed /= 1.2f;
	}
}

//************************************************************************************
//Function:
glm::mat4 CCamera::getPrevViewMatrix() const
{
	return glm::lookAt(m_PrevCameraPos, m_PrevCameraPos + m_PrevCameraFront, m_UpVector);
}