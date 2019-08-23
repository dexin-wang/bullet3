//
//  btDeformableMultiBodyConstraintSolver.h
//  BulletSoftBody
//
//  Created by Xuchen Han on 8/22/19.
//

#ifndef BT_DEFORMABLE_MULTIBODY_CONSTRAINT_SOLVER_H
#define BT_DEFORMABLE_MULTIBODY_CONSTRAINT_SOLVER_H

#include "btDeformableBodySolver.h"
#include "BulletDynamics/Featherstone/btMultiBodyConstraintSolver.h"

class btDeformableBodySolver;

ATTRIBUTE_ALIGNED16(class)
btDeformableMultiBodyConstraintSolver : public btMultiBodyConstraintSolver
{
    btDeformableBodySolver* m_deformableSolver;
    
protected:
    // override the iterations method to include deformable/multibody contact
    virtual btScalar solveGroupCacheFriendlyIterations(btCollisionObject** bodies,int numBodies,btPersistentManifold** manifoldPtr, int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* debugDrawer);
    
    void solverBodyWriteBack(const btContactSolverInfo& infoGlobal)
    {
        for (int i = 0; i < m_tmpSolverBodyPool.size(); i++)
        {
            btRigidBody* body = m_tmpSolverBodyPool[i].m_originalBody;
            if (body)
            {
                m_tmpSolverBodyPool[i].m_originalBody->setLinearVelocity(m_tmpSolverBodyPool[i].m_linearVelocity + m_tmpSolverBodyPool[i].m_deltaLinearVelocity);
                m_tmpSolverBodyPool[i].m_originalBody->setAngularVelocity(m_tmpSolverBodyPool[i].m_angularVelocity+m_tmpSolverBodyPool[i].m_deltaAngularVelocity);
            }
        }
    }
    
    void writeToSolverBody(btCollisionObject** bodies, int numBodies, const btContactSolverInfo& infoGlobal)
    {
        btSISolverSingleIterationData siData(m_tmpSolverBodyPool,
                                             m_tmpSolverContactConstraintPool,
                                             m_tmpSolverNonContactConstraintPool,
                                             m_tmpSolverContactFrictionConstraintPool,
                                             m_tmpSolverContactRollingFrictionConstraintPool,
                                             m_orderTmpConstraintPool,
                                             m_orderNonContactConstraintPool,
                                             m_orderFrictionConstraintPool,
                                             m_tmpConstraintSizesPool,
                                             m_resolveSingleConstraintRowGeneric,
                                             m_resolveSingleConstraintRowLowerLimit,
                                             m_resolveSplitPenetrationImpulse,
                                             m_kinematicBodyUniqueIdToSolverBodyTable,
                                             m_btSeed2,
                                             m_fixedBodyId,
                                             m_maxOverrideNumSolverIterations);
        
        for (int i = 0; i < numBodies; i++)
        {
            int bodyId = siData.getOrInitSolverBody(*bodies[i], infoGlobal.m_timeStep);
            
            btRigidBody* body = btRigidBody::upcast(bodies[i]);
            if (body && body->getInvMass())
            {
                btSolverBody& solverBody = siData.m_tmpSolverBodyPool[bodyId];
                solverBody.m_linearVelocity = body->getLinearVelocity() - solverBody.m_deltaLinearVelocity;
                solverBody.m_angularVelocity = body->getAngularVelocity() - solverBody.m_deltaAngularVelocity;
            }
        }
    }
    
public:
    BT_DECLARE_ALIGNED_ALLOCATOR();
    
    void setDeformableSolver(btDeformableBodySolver* deformableSolver)
    {
        m_deformableSolver = deformableSolver;
    }
    
    virtual void solveMultiBodyGroup(btCollisionObject * *bodies, int numBodies, btPersistentManifold** manifold, int numManifolds, btTypedConstraint** constraints, int numConstraints, btMultiBodyConstraint** multiBodyConstraints, int numMultiBodyConstraints, const btContactSolverInfo& info, btIDebugDraw* debugDrawer, btDispatcher* dispatcher)
    {
        m_tmpMultiBodyConstraints = multiBodyConstraints;
        m_tmpNumMultiBodyConstraints = numMultiBodyConstraints;
        
        // inherited from MultiBodyConstraintSolver
        solveGroupCacheFriendlySetup(bodies, numBodies, manifold, numManifolds, constraints, numConstraints, info, debugDrawer);
        
        // overriden
        solveGroupCacheFriendlyIterations(bodies, numBodies, manifold, numManifolds, constraints, numConstraints, info, debugDrawer);
        
        // inherited from MultiBodyConstraintSolver
        solveGroupCacheFriendlyFinish(bodies, numBodies, info);
        
        m_tmpMultiBodyConstraints = 0;
        m_tmpNumMultiBodyConstraints = 0;
    }
};

#endif /* BT_DEFORMABLE_MULTIBODY_CONSTRAINT_SOLVER_H */
