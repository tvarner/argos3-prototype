/**
 * @file <argos3/plugins/robot/prototype/simulator/entities/electromagnet_entity.cpp>
 *
 * @author Michael Allwright - <allsey87@gmail.com>
 */

#include "electromagnet_entity.h"
#include <argos3/core/simulator/space/space.h>

namespace argos {

   /****************************************/
   /****************************************/

   CElectromagnetEntity::CElectromagnetEntity(CComposableEntity* pc_parent) :
      CEntity(pc_parent) {}

   /****************************************/
   /****************************************/

   CElectromagnetEntity::CElectromagnetEntity(CComposableEntity* pc_parent,
                                              const std::string& str_id,
                                              const CVector3& c_passive_field,
                                              const CVector3& c_active_field) :
      CEntity(pc_parent, str_id),
      m_cField(c_passive_field),
      m_cPassiveField(c_passive_field),
      m_cActiveField(c_active_field) {}

   /****************************************/
   /****************************************/

   void CElectromagnetEntity::Init(TConfigurationNode& t_tree) {
      try {
         /* Parse XML */
         CEntity::Init(t_tree);
         GetNodeAttribute(t_tree, "passive_field", m_cPassiveField);
         GetNodeAttributeOrDefault(t_tree, "active_field", m_cActiveField, CVector3::ZERO);
         m_cField = m_cPassiveField;
      }
      catch(CARGoSException& ex) {
         THROW_ARGOSEXCEPTION_NESTED("Error while initializing electromagnet entity", ex);
      }
   }

   /****************************************/
   /****************************************/

   void CElectromagnetEntity::Reset() {
      m_cField = m_cPassiveField;
   }

   /****************************************/
   /****************************************/

   REGISTER_STANDARD_SPACE_OPERATIONS_ON_ENTITY(CElectromagnetEntity);

   /****************************************/
   /****************************************/

}
