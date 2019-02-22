#ifndef __C_LDEBUG_IK_HPP__
#define __C_LDEBUG_IK_HPP__
/*
#include <BussIK/LinearR3.h>
#include <BussIK/Node.h>
#include <BussIK/Tree.h>
#include <BussIK/Jacobian.h>
#include <BussIK/VectorRn.h>
#include <Bullet3Common/b3AlignedObjectArray.h>
#include "pragma/debug/c_debugoverlay.h"

///quick demo showing the right-handed coordinate system and positive rotations around each axis
class InverseKinematicsExample
{
	int m_ikMethod;
	Tree m_ikTree;
	std::vector<Node*> m_ikNodes;
	Jacobian* m_ikJacobian;

 	std::vector<int> m_movingInstances;
	int m_targetInstance;
	enum
	{
		numCubesX = 20,
		numCubesY = 20
	};
	std::unordered_map<uint32_t,std::shared_ptr<DebugRenderer::BaseObject>> m_dbgObjects;
public:
    
    InverseKinematicsExample(int option);
    virtual ~InverseKinematicsExample();

    
    virtual void physicsDebugDraw(int debugDrawMode);
    virtual void    initPhysics();
    virtual void    exitPhysics();

	void BuildKukaIIWAShape();

	void getLocalTransform(const Node* node, btTransform& act);
	void MyDrawTree(Node* node, const btTransform& tr);
    virtual void	stepSimulation(float deltaTime);
    virtual void	renderScene();

	
    virtual void	physicsDebugDraw();
    virtual bool	mouseMoveCallback(float x,float y);
    virtual bool	mouseButtonCallback(int button, int state, float x, float y);
    virtual bool	keyboardCallback(int key, int state);
    
	virtual void resetCamera();

};
*/
#endif
