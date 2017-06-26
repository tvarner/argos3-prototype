/**
 * @file <argos3/plugins/robots/prototype/simulator/prototype/actuators/prototype_leds_default_actuator.cpp>
 *
 * @author Michael Allwright - <allsey87@gmail.com>
 */

#include "prototype_leds_default_actuator.h"
#include <argos3/core/simulator/simulator.h>
#include <argos3/core/utility/logging/argos_log.h>
#include <argos3/plugins/simulator/media/led_medium.h>

namespace argos {

   /****************************************/
   /****************************************/

   CPrototypeLEDsDefaultActuator::CPrototypeLEDsDefaultActuator() :
      m_pcLEDEquippedEntity(NULL) {}

   /****************************************/
   /****************************************/

   void CPrototypeLEDsDefaultActuator::SetRobot(CComposableEntity& c_entity) {
      m_pcLEDEquippedEntity = &(c_entity.GetComponent<CPrototypeLEDEquippedEntity>("leds"));
      m_tSettings.resize(m_pcLEDEquippedEntity->GetAllLEDs().size());
      m_pcLEDEquippedEntity->SetCanBeEnabledIfDisabled(true);
      m_pcLEDEquippedEntity->Enable();
   }

   /****************************************/
   /****************************************/

   void CPrototypeLEDsDefaultActuator::Init(TConfigurationNode& t_tree) {
      try {
         CCI_LEDsActuator::Init(t_tree);
      }
      catch(CARGoSException& ex) {
         THROW_ARGOSEXCEPTION_NESTED("Error initializing the LEDs default actuator", ex);
      }
   }

   /****************************************/
   /****************************************/

   void CPrototypeLEDsDefaultActuator::Update() {
      m_pcLEDEquippedEntity->SetAllLEDsColors(m_tSettings);
   }

   /****************************************/
   /****************************************/

   void CPrototypeLEDsDefaultActuator::Reset() {
      SetAllColors(CColor::BLACK);
   }

   /****************************************/
   /****************************************/

   void CPrototypeLEDsDefaultActuator::Destroy() {
   }

   /****************************************/
   /****************************************/

}

REGISTER_ACTUATOR(CPrototypeLEDsDefaultActuator,
                  "pleds", "default",
                  "Michael Allwright [allsey87@gmail.com]",
                  "1.0",
                  "The Prototype LEDs actuator.",
                  "This actuator controls a group of LEDs. For a complete description of its\n"
                  "usage, refer to the ci_leds_actuator.h file.\n\n"
                  "REQUIRED XML CONFIGURATION\n\n"
                  "  <controllers>\n"
                  "    ...\n"
                  "    <my_controller ...>\n"
                  "      ...\n"
                  "      <actuators>\n"
                  "        ...\n"
                  "        <leds implementation=\"default\"/>"
                  "        ...\n"
                  "      </actuators>\n"
                  "      ...\n"
                  "    </my_controller>\n"
                  "    ...\n"
                  "  </controllers>\n\n"
                  "The 'medium' attribute sets the id of the LED medium declared in the <media>\n"
                  "XML section.\n\n"
                  "OPTIONAL XML CONFIGURATION\n\n"
                  "None.\n",
                  "Usable"
   );

