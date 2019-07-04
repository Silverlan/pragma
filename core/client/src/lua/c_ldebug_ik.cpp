#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/c_ldebug_ik.hpp"
#include "pragma/debug/c_debugoverlay.h"
#include <pragma/physics/environment.hpp>

extern DLLCLIENT CGame *c_game;
//
//#define RADIAN(X)	((X)*RadiansToDegrees)
//
//#define MAX_NUM_NODE	1000
//#define MAX_NUM_THETA	1000
//#define MAX_NUM_EFFECT	100
//
//enum Method {IK_JACOB_TRANS=0, IK_PURE_PSEUDO, IK_DLS, IK_SDLS , IK_DLS_SVD};
//
//double T = 0;
//VectorR3 targetaa[MAX_NUM_EFFECT];
//
//
//
//// Make slowdown factor larger to make the simulation take larger, less frequent steps
//// Make the constant factor in Tstep larger to make time pass more quickly
////const int SlowdownFactor = 40;
//const int SlowdownFactor = 0;		// Make higher to take larger steps less frequently
//const int SleepsPerStep=SlowdownFactor;
//int SleepCounter=0;
////const double Tstep = 0.0005*(double)SlowdownFactor;		// Time step
//
//
//int	AxesList;		/* list to hold the axes		*/
//int	AxesOn;			/* ON or OFF				*/
//
//float	Scale, Scale2;		/* scaling factors			*/
//
//
//
//int JointLimitsOn;
//int RestPositionOn;
//int UseJacobianTargets1;
//
//
//int numIteration = 1;
//double error = 0.0;
//double errorDLS = 0.0;
//double errorSDLS = 0.0;
//double sumError = 0.0;
//double sumErrorDLS = 0.0;
//double sumErrorSDLS = 0.0;
//
//#ifdef _DYNAMIC
//bool initMaxDist = true;
//extern double Excess[];
//extern double dsnorm[];
//#endif
//
//
//
//
//void Reset(Tree &tree, Jacobian* m_ikJacobian)
//{
//	AxesOn = false;
//	
//	Scale  = 1.0;
//	Scale2 = 0.0;		/* because add 1. to it in Display()	*/
//	
//	JointLimitsOn = true;
//	RestPositionOn = false;
//	UseJacobianTargets1 = false;
//	
//	                  
//	tree.Init();
//	tree.Compute();
//	m_ikJacobian->Reset();
//
//}
//
//// Update target positions
//
//void UpdateTargets( double T2, Tree & treeY) {
//	double T = T2 / 5.;
//	targetaa[0].Set(0.6*b3Sin(0), 0.6*b3Cos(0), 0.5+0.4*b3Sin(3 * T));
//}
//
//
//// Does a single update (on one kind of tree)
//void DoUpdateStep(double Tstep, Tree & treeY, Jacobian *jacob, int ikMethod) {
//	
//	if ( SleepCounter==0 ) {
//		T += Tstep;
//		UpdateTargets( T , treeY);
//	} 
//
//	if ( UseJacobianTargets1 ) {
//		jacob->SetJtargetActive();
//	}
//	else {
//		jacob->SetJendActive();
//	}
//	jacob->ComputeJacobian(targetaa);						// Set up Jacobian and deltaS vectors
//
//	// Calculate the change in theta values 
//	switch (ikMethod) {
//		case IK_JACOB_TRANS:
//			jacob->CalcDeltaThetasTranspose();		// Jacobian transpose method
//			break;
//		case IK_DLS:
//			jacob->CalcDeltaThetasDLS();			// Damped least squares method
//			break;
//        case IK_DLS_SVD:
//            jacob->CalcDeltaThetasDLSwithSVD();
//            break;
//		case IK_PURE_PSEUDO:
//			jacob->CalcDeltaThetasPseudoinverse();	// Pure pseudoinverse method
//			break;
//		case IK_SDLS:
//			jacob->CalcDeltaThetasSDLS();			// Selectively damped least squares method
//			break;
//		default:
//			jacob->ZeroDeltaThetas();
//			break;
//	}
//
//	if ( SleepCounter==0 ) {
//		jacob->UpdateThetas();							// Apply the change in the theta values
//		jacob->UpdatedSClampValue(targetaa);
//		SleepCounter = SleepsPerStep;
//	}
//	else { 
//		SleepCounter--;
//	}
//
//
//}
//
//void InverseKinematicsExample::BuildKukaIIWAShape()
//{
//	//const VectorR3& unitx = VectorR3::UnitX;
//	const VectorR3& unity = VectorR3::UnitY;
//	const VectorR3& unitz = VectorR3::UnitZ;
//	const VectorR3 unit1(sqrt(14.0) / 8.0, 1.0 / 8.0, 7.0 / 8.0);
//	const VectorR3& zero = VectorR3::Zero;
//
//	float minTheta = -4 * PI;
//	float maxTheta = 4 * PI;
//
//	m_ikNodes.resize(8);//7DOF+additional endeffector
//
//	m_ikNodes[0] = new Node(VectorR3(0.100000, 0.000000, 0.087500), unitz, 0.08, JOINT, -1e30, 1e30, RADIAN(0.));
//	m_ikTree.InsertRoot(m_ikNodes[0]);
//
//	m_ikNodes[1] = new Node(VectorR3(0.100000, -0.000000, 0.290000), unity, 0.08, JOINT, -0.5, 0.4, RADIAN(0.));
//	m_ikTree.InsertLeftChild(m_ikNodes[0], m_ikNodes[1]);
//
//	m_ikNodes[2] = new Node(VectorR3(0.100000, -0.000000, 0.494500), unitz, 0.08, JOINT, minTheta, maxTheta, RADIAN(0.));
//	m_ikTree.InsertLeftChild(m_ikNodes[1], m_ikNodes[2]);
//
//	m_ikNodes[3] = new Node(VectorR3(0.100000, 0.000000, 0.710000), -unity, 0.08, JOINT, minTheta, maxTheta, RADIAN(0.));
//	m_ikTree.InsertLeftChild(m_ikNodes[2], m_ikNodes[3]);
//
//	m_ikNodes[4] = new Node(VectorR3(0.100000, 0.000000, 0.894500), unitz, 0.08, JOINT, minTheta, maxTheta, RADIAN(0.));
//	m_ikTree.InsertLeftChild(m_ikNodes[3], m_ikNodes[4]);
//
//	m_ikNodes[5] = new Node(VectorR3(0.100000, 0.000000, 1.110000), unity, 0.08, JOINT, minTheta, maxTheta, RADIAN(0.));
//	m_ikTree.InsertLeftChild(m_ikNodes[4], m_ikNodes[5]);
//
//	m_ikNodes[6] = new Node(VectorR3(0.100000, 0.000000, 1.191000), unitz, 0.08, JOINT, minTheta, maxTheta, RADIAN(0.));
//	m_ikTree.InsertLeftChild(m_ikNodes[5], m_ikNodes[6]);
//
//	m_ikNodes[7] = new Node(VectorR3(0.100000, 0.000000, 1.20000), zero, 0.08, EFFECTOR);
//	m_ikTree.InsertLeftChild(m_ikNodes[6], m_ikNodes[7]);
//
//}
//
//
//DLLCLIENT InverseKinematicsExample*    InverseKinematicsExampleCreateFunc(int option)
//{
//	return new InverseKinematicsExample(option);
//}
//
//InverseKinematicsExample::InverseKinematicsExample(int option)
//:
//m_ikMethod(option),
//m_targetInstance(-1)
//{
//
//}
//InverseKinematicsExample::~InverseKinematicsExample()
//{
//}
//
//    
//void InverseKinematicsExample::physicsDebugDraw(int debugDrawMode)
//{
//        
//}
//void    InverseKinematicsExample::initPhysics()
//{
//	BuildKukaIIWAShape();
//	m_ikJacobian = new Jacobian(&m_ikTree);
//	Reset(m_ikTree,m_ikJacobian);
//}
//void    InverseKinematicsExample::exitPhysics()
//{
//	delete m_ikJacobian;
//	m_ikJacobian = 0;
//}
//
//void InverseKinematicsExample::getLocalTransform(const Node* node, btTransform& act)
//{
//	btVector3 axis = btVector3(node->v.x, node->v.y, node->v.z);
//	btQuaternion rot(0, 0, 0, 1);
//	if (axis.length())
//	{
//		rot = btQuaternion (axis, node->theta);
//	}
//	act.setIdentity();
//	act.setRotation(rot);
//	act.setOrigin(btVector3(node->r.x, node->r.y, node->r.z));
//}
//void InverseKinematicsExample::MyDrawTree(Node* node, const btTransform& tr)
//{
//	btVector3 lineColor = btVector3(0, 0, 0);
//	int lineWidth = 2;
//	auto fUpdateLine = [this](int32_t tIdx,const Vector3 &start,const Vector3 &end,const Color &col) {
//		/*auto it = m_dbgObjects.find(tIdx);
//		if(it == m_dbgObjects.end())
//			it = m_dbgObjects.insert(std::make_pair(tIdx,DebugRenderer::DrawLine(start,end,col))).first;
//		auto &wo = static_cast<DebugRenderer::WorldObject&>(*m_dbgObjects.at(tIdx));
//		wo.GetVertices().at(0) = start;
//		wo.GetVertices().at(1) = end;
//		wo.UpdateVertexBuffer();*/
//		DebugRenderer::DrawLine(start,end,col,0.05f);
//	};
//	if (node != 0) {
//	//	glPushMatrix();
//		btVector3 pos = btVector3(tr.getOrigin().x(), tr.getOrigin().y(), tr.getOrigin().z());
//		btVector3 color = btVector3(0, 1, 0);
//		int pointSize = 10;
//		auto enPos = uvec::create(pos /PhysEnv::WORLD_SCALE);
//		auto it = m_dbgObjects.find(0u);
//		if(it == m_dbgObjects.end())
//			it = m_dbgObjects.insert(std::make_pair(0u,DebugRenderer::DrawPoint(enPos,Color::Lime))).first;
//		it->second->SetPos(enPos);
//
//		auto enForward = uvec::create((tr.getBasis().getColumn(0)) /PhysEnv::WORLD_SCALE);
//		auto enRight = uvec::create((tr.getBasis().getColumn(1)) /PhysEnv::WORLD_SCALE);
//		auto enUp = uvec::create((tr.getBasis().getColumn(2)) /PhysEnv::WORLD_SCALE);
//		fUpdateLine(1,enPos,enPos +enForward *static_cast<float>(0.05f /PhysEnv::WORLD_SCALE),Color::Red);
//		fUpdateLine(2,enPos,enPos +enRight *static_cast<float>(0.05f /PhysEnv::WORLD_SCALE),Color::Lime);
//		fUpdateLine(3,enPos,enPos +enUp *static_cast<float>(0.05f /PhysEnv::WORLD_SCALE),Color::Aqua);
//			
//		btVector3 axisLocal = btVector3(node->v.x, node->v.y, node->v.z);
//		btVector3 axisWorld = tr.getBasis()*axisLocal;
//
//		fUpdateLine(4,enPos,enPos +0.1f *uvec::create(axisWorld),Color::Yellow);
//
//		//node->DrawNode(node == root);	// Recursively draw node and update ModelView matrix
//		if (node->left) {
//			btTransform act;
//			getLocalTransform(node->left, act);
//				
//			btTransform trl = tr*act;
//			auto trOrigin = uvec::create(tr.getOrigin() /PhysEnv::WORLD_SCALE);
//			auto trlOrigin = uvec::create(trl.getOrigin() /PhysEnv::WORLD_SCALE);
//			fUpdateLine(5,trOrigin,trlOrigin,Color::Maroon);
//			MyDrawTree(node->left, trl);		// Draw tree of children recursively
//		}
//	//	glPopMatrix();
//		if (node->right) {
//			btTransform act;
//			getLocalTransform(node->right, act);
//			btTransform trr = tr*act;
//			auto trOrigin = uvec::create(tr.getOrigin() /PhysEnv::WORLD_SCALE);
//			auto trrOrigin = uvec::create(trr.getOrigin() /PhysEnv::WORLD_SCALE);
//			fUpdateLine(6,trOrigin,trrOrigin,Color::Silver);
//			MyDrawTree(node->right,trr);		// Draw right siblings recursively
//		}
//	}
//
//}
//void	InverseKinematicsExample::renderScene()
//{
//		
//		
//	btTransform act;
//	getLocalTransform(m_ikTree.GetRoot(), act);
//	MyDrawTree(m_ikTree.GetRoot(), act);
//		
//	btVector3 pos = btVector3(targetaa[0].x, targetaa[0].y, targetaa[0].z);
//	btQuaternion orn(0, 0, 0, 1);
//
//	//m_app->m_renderer->writeSingleInstanceTransformToCPU(pos, orn, m_targetInstance);
//	//m_app->m_renderer->writeTransforms();
//	//m_app->m_renderer->renderScene();
//}
//void	InverseKinematicsExample::stepSimulation(float deltaTime)
//{
//	DoUpdateStep(deltaTime, m_ikTree, m_ikJacobian, m_ikMethod);
//}
//
//	
//void	InverseKinematicsExample::physicsDebugDraw()
//{
//      		
//}
//bool	InverseKinematicsExample::mouseMoveCallback(float x,float y)
//{
//	return false;   
//}
//bool	InverseKinematicsExample::mouseButtonCallback(int button, int state, float x, float y)
//{
//    return false;   
//}
//bool	InverseKinematicsExample::keyboardCallback(int key, int state)
//{
//    return false;   
//}
//    
//void InverseKinematicsExample::resetCamera()
//{
//	float dist = 1.3;
//	float pitch = -13;
//	float yaw = 120;
//	float targetPos[3]={-0.35,0.14,0.25};
//	/*if (m_app->m_renderer  && m_app->m_renderer->getActiveCamera())
//	{
//			
//		m_app->m_renderer->getActiveCamera()->setCameraDistance(dist);
//		m_app->m_renderer->getActiveCamera()->setCameraPitch(pitch);
//		m_app->m_renderer->getActiveCamera()->setCameraYaw(yaw);
//		m_app->m_renderer->getActiveCamera()->setCameraTargetPosition(targetPos[0],targetPos[1],targetPos[2]);
//	}*/
//}
//
