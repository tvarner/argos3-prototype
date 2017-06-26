/**
 * @file <argos3/plugins/robots/prototype/simulator/entities/joint_equipped_entity.h>
 *
 * @author Michael Allwright - <allsey87@gmail.com>
 */

#ifndef JOINT_EQUIPPED_ENTITY_H
#define JOINT_EQUIPPED_ENTITY_H

namespace argos {
   class CJointEquippedEntity;
   class CJointEntity;
}

#include <argos3/plugins/robots/prototype/simulator/entities/joint_entity.h>
#include <argos3/core/simulator/entity/composable_entity.h>
#include <map>

namespace argos {

   class CJointEquippedEntity : public CComposableEntity {

   public:

      ENABLE_VTABLE();

      typedef std::map<std::string, CJointEquippedEntity*> TMap;

   public:

      CJointEquippedEntity(CComposableEntity* pc_parent);

      CJointEquippedEntity(CComposableEntity* pc_parent,
                          const std::string& str_id);

      virtual void Init(TConfigurationNode& t_tree);
      virtual void Reset();
      virtual void Update() {}

      CJointEntity& GetJoint(UInt32 un_index);

      inline CJointEntity::TList& GetAllJoints() {
         return m_tJoints;
      }

      void SetJointPosition(UInt32 un_index,
                           const CVector3& c_position);

      void SetJointOrientation(UInt32 un_index,
                              const CQuaternion& c_orientation);

      virtual std::string GetTypeDescription() const {
         return "joints";
      }

   protected:

      virtual void UpdateComponents();

   protected:

      CJointEntity::TList m_tJoints;
   };

}

#endif
