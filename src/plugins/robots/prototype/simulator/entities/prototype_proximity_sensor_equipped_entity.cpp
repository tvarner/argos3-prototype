
/**
 * @file <argos3/plugins/robots/prototype/simulator/entities/prototype_proximity_sensor_equipped_entity.cpp>
 *
 * @author Michael Allwright <allsey@gmail.com>
 */
#include "prototype_proximity_sensor_equipped_entity.h"
#include <argos3/core/simulator/space/space.h>

namespace argos {

   /****************************************/
   /****************************************/

   CPrototypeProximitySensorEquippedEntity::CPrototypeProximitySensorEquippedEntity(CComposableEntity* pc_parent) :
      CEntity(pc_parent) {
      SetCanBeEnabledIfDisabled(false);
   }
   
   /****************************************/
   /****************************************/

   CPrototypeProximitySensorEquippedEntity::CPrototypeProximitySensorEquippedEntity(CComposableEntity* pc_parent,
                                                                                    const std::string& str_id) :
      CEntity(pc_parent, str_id) {
      SetCanBeEnabledIfDisabled(false);
   }

   /****************************************/
   /****************************************/

   CPrototypeProximitySensorEquippedEntity::~CPrototypeProximitySensorEquippedEntity() {
      while(! m_tSensors.empty()) {
         delete m_tSensors.back();
         m_tSensors.pop_back();
      }
   }

   /****************************************/
   /****************************************/

   void CPrototypeProximitySensorEquippedEntity::Init(TConfigurationNode& t_tree) {
      try {
         /*
          * Parse basic entity stuff
          */
         CEntity::Init(t_tree);
         /*
          * Parse proximity sensors
          */
         /* Not adding any sensor is a fatal error */
         if(t_tree.NoChildren()) {
            THROW_ARGOSEXCEPTION("No sensors defined");
         }
         /* Go through children */
         TConfigurationNodeIterator it;
         for(it = it.begin(&t_tree); it != it.end(); ++it) {
            
            std::string strSensorBody;
            GetNodeAttribute(*it, "body", strSensorBody);
            CBodyEntity& cSensorBody = GetParent().GetComponent<CBodyEntity>("bodies.body[" + strSensorBody + "]");
            if(it->Value() == "sensor") {
               CVector3 cOff, cDir;
               Real fRange;
               GetNodeAttribute(*it, "offset", cOff);
               GetNodeAttribute(*it, "direction", cDir);
               GetNodeAttribute(*it, "range", fRange);
               AddSensor(&cSensorBody.GetPositionalEntity(), cOff, cDir, fRange);
            }
            else if(it->Value() == "ring") {
               CVector3 cRingCenter;
               GetNodeAttributeOrDefault(t_tree, "center", cRingCenter, cRingCenter);
               Real fRadius;
               GetNodeAttribute(t_tree, "radius", fRadius);
               CDegrees cRingStartAngleDegrees;
               GetNodeAttributeOrDefault(t_tree, "start_angle", cRingStartAngleDegrees, cRingStartAngleDegrees);
               CRadians cRingStartAngleRadians = ToRadians(cRingStartAngleDegrees);
               Real fRange;
               GetNodeAttribute(t_tree, "range", fRange);
               UInt32 unNumSensors;
               GetNodeAttribute(t_tree, "num_sensors", unNumSensors);
               AddSensorRing(&cSensorBody.GetPositionalEntity(),
                             cRingCenter,
                             fRadius,
                             cRingStartAngleRadians,
                             fRange,
                             unNumSensors);
            }
            else {
               THROW_ARGOSEXCEPTION("Unrecognized tag \"" << it->Value() << "\"");
            }
         }
      }
      catch(CARGoSException& ex) {
         THROW_ARGOSEXCEPTION_NESTED("Initialization error in proximity sensor equipped entity", ex);
      }
   }

   /****************************************/
   /****************************************/

   void CPrototypeProximitySensorEquippedEntity::AddSensor(CPositionalEntity* pc_position,
                                                  const CVector3& c_offset,
                                                  const CVector3& c_direction,
                                                  Real f_range) {
      m_tSensors.push_back(new SSensor(pc_position, c_offset, c_direction, f_range));
   }

   /****************************************/
   /****************************************/
   
   void CPrototypeProximitySensorEquippedEntity::AddSensorRing(CPositionalEntity* pc_position,
                                                      const CVector3& c_center,
                                                      Real f_radius,
                                                      const CRadians& c_start_angle,
                                                      Real f_range,
                                                      UInt32 un_num_sensors) {
      CRadians cSensorSpacing = CRadians::TWO_PI / un_num_sensors;
      CRadians cAngle;
      CVector3 cOff, cDir;
      for(UInt32 i = 0; i < un_num_sensors; ++i) {
         cAngle = c_start_angle + i * cSensorSpacing;
         cAngle.SignedNormalize();
         cOff.Set(f_radius, 0.0f, 0.0f);
         cOff.RotateZ(cAngle);
         cOff += c_center;
         cDir.Set(f_range, 0.0f, 0.0f);
         cDir.RotateZ(cAngle);
         AddSensor(pc_position, cOff, cDir, f_range);
      }
   }

   /****************************************/
   /****************************************/

   REGISTER_STANDARD_SPACE_OPERATIONS_ON_ENTITY(CPrototypeProximitySensorEquippedEntity);
   
   /****************************************/
   /****************************************/

}
