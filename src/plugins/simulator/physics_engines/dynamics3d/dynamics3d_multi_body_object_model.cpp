
#include "dynamics3d_multi_body_object_model.h"
#include <argos3/core/simulator/entity/composable_entity.h>

namespace argos {

   /****************************************/
   /****************************************/

   CDynamics3DMultiBodyObjectModel::CDynamics3DMultiBodyObjectModel(CDynamics3DEngine& c_engine,
                                                                    ComposableEntity& c_entity) :
      CDynamics3DModel(c_engine, c_entity.GetComponent<CEmbodiedEntity>("body")),
      m_cEntity(c_entity),
      m_cModel(0, 0, btVector3(0,0,0), false, false) {}

   /****************************************/
   /****************************************/

   CDynamics3DMultiBodyObjectModel::~CDynamics3DMultiBodyObjectModel() {
   }

   /****************************************/
   /****************************************/

   void CDynamics3DMultiBodyObjectModel::MoveTo(const CVector3& c_position,
                                                const CQuaternion& c_orientation) {
      /* Transform coordinate systems and move the body */
      m_cMotionState.m_graphicsWorldTrans =
         btTransform(btQuaternion(c_orientation.GetX(),
                                  c_orientation.GetZ(), 
                                 -c_orientation.GetY(),
                                  c_orientation.GetW()),
                     btVector3(c_position.GetX(),
                               c_position.GetZ(),
                               -c_position.GetY()));
      /* Update body */
      m_cBody.setMotionState(&m_cMotionState);
      m_cBody.activate();
      /* Update the bounding box */
      CalculateBoundingBox();
      /* Update ARGoS entity state */
      CDynamics3DModel::UpdateEntityStatus();
   }

   /****************************************/
   /****************************************/

   void CDynamics3DMultiBodyObjectModel::Reset() {
      /* Reset body position */
      const CVector3& cPosition = GetEmbodiedEntity().GetOriginAnchor().Position;
      const CQuaternion& cOrientation = GetEmbodiedEntity().GetOriginAnchor().Orientation;
      m_cMotionState.m_graphicsWorldTrans =
         btTransform(btQuaternion(cOrientation.GetX(),
                                  cOrientation.GetZ(), 
                                 -cOrientation.GetY(),
                                  cOrientation.GetW()),
                     btVector3(cPosition.GetX(),
                               cPosition.GetZ(),
                              -cPosition.GetY()));
      /* setup the rigid body */
      GetEngine().GetPhysicsWorld()->removeRigidBody(&m_cBody);
      
      m_cBody = btRigidBody(btRigidBody::btRigidBodyConstructionInfo(m_fMass,
                                                                     &m_cMotionState,
                                                                     m_pcShape,
                                                                     m_cInertia));

      GetEngine().GetPhysicsWorld()->addRigidBody(&m_cBody);
      /* set the default surface friction */
      m_cBody.setFriction(0.5f);
      /* For reverse look up */
      m_cBody.setUserPointer(this);
      /* Activate the body */
      m_cBody.activate();
      /* Update bounding box */
      CalculateBoundingBox();
   }

   /****************************************/
   /****************************************/

   void CDynamics3DMultiBodyObjectModel::CalculateBoundingBox() {
      btVector3 cAabbMin;
      btVector3 cAabbMax;    
      /* Get the axis aligned bounding box for the current body */
      m_pcShape->getAabb(m_cBody.getWorldTransform(), cAabbMin, cAabbMax);
      /* Write back the bounding box swapping the coordinate systems and the Y component */
      GetBoundingBox().MinCorner.Set(cAabbMin.getX(), -cAabbMax.getZ(), cAabbMin.getY());
      GetBoundingBox().MaxCorner.Set(cAabbMax.getX(), -cAabbMin.getZ(), cAabbMax.getY());
   }
  
   /****************************************/
   /****************************************/

   bool CDynamics3DMultiBodyObjectModel::IsCollidingWithSomething() const {
      /* get the collision dispatcher */
      const btCollisionDispatcher* pcCollisionDispatcher =
         GetEngine().GetCollisionDispatcher();
      /* for each manifold from the collision dispatcher */
      for(UInt32 i = 0; i < UInt32(pcCollisionDispatcher->getNumManifolds()); i++) {
         const btPersistentManifold* pcContactManifold =
            pcCollisionDispatcher->getManifoldByIndexInternal(i);
         const CDynamics3DModel* pcModelA =
            static_cast<const CDynamics3DModel*>(pcContactManifold->getBody0()->getUserPointer());
         const CDynamics3DModel* pcModelB =
            static_cast<const CDynamics3DModel*>(pcContactManifold->getBody1()->getUserPointer());
         /* ignore collisions of bodies that don't belong to a model (e.g. the ground) */
         if((pcModelA == NULL) || (pcModelB == NULL)) {
            continue;
         }
         /* check that the collision involves this model */
         if((pcModelA == this) || (pcModelB == this)) {
            /* One of the two bodies involved in the contact manifold belongs to this model,
               check for contact points with negative distance to indicate a collision */
            for(UInt32 j = 0; j < UInt32(pcContactManifold->getNumContacts()); j++) {  
               const btManifoldPoint& cManifoldPoint = pcContactManifold->getContactPoint(j);
               if (cManifoldPoint.getDistance() < 0.0f) {
                  return true;
               }
            }
         }
      }
      return false;
   }

   /****************************************/
   /****************************************/

   void CDynamics3DMultiBodyObjectModel::SetBody() {
      /* setup the multi body model */
      m_cMultiBody = btRigidBody(btRigidBody::btRigidBodyConstructionInfo(m_fMass,
                                                                     &m_cMotionState,
                                                                     m_pcShape,
                                                                     m_cInertia));

      m_cMultiBody.finalizeMultiDof();

      /* set the default surface friction */
      m_cMultiBody.setFriction(0.5f);
      /* For reverse look up */
      m_cBody.setUserPointer(this);
      /* Add body to world */
      GetEngine().GetPhysicsWorld()->addMultiBody(m_cMultiBody);
      /* Register the origin anchor update method */
      RegisterAnchorMethod(GetEmbodiedEntity().GetOriginAnchor(),
                           &CDynamics3DMultiBodyObjectModel::UpdateOriginAnchor);
      /* Calculate the bounding box */
      CalculateBoundingBox();
   }

   /****************************************/
   /****************************************/

   void CDynamics3DMultiBodyObjectModel::UpdateOriginAnchor(SAnchor& s_anchor) {
   }

   /****************************************/
   /****************************************/

}