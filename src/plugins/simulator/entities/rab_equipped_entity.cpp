/**
 * @file <argos3/core/simulator/entity/rab_equipped_entity.cpp>
 *
 * @author Carlo Pinciroli - <ilpincy@gmail.com>
 */

#include "rab_equipped_entity.h"
#include <argos3/core/utility/string_utilities.h>
#include <argos3/core/simulator/simulator.h>
#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/entity/composable_entity.h>

namespace argos {

   /****************************************/
   /****************************************/

   CRABEquippedEntity::CRABEquippedEntity(CComposableEntity* pc_parent) :
      CPositionalEntity(pc_parent),
      m_psAnchor(NULL),
      m_fRange(0.0f) {
      Disable();
      SetCanBeEnabledIfDisabled(false);
   }

   /****************************************/
   /****************************************/

   CRABEquippedEntity::CRABEquippedEntity(CComposableEntity* pc_parent,
                                          const std::string& str_id,
                                          size_t un_msg_size,
                                          Real f_range,
                                          const SAnchor& s_anchor,
                                          const CVector3& c_pos_offset,
                                          const CQuaternion& c_rot_offset) :
      CPositionalEntity(pc_parent,
                        str_id),
      m_psAnchor(&s_anchor),
      m_cPosOffset(c_pos_offset),
      m_cRotOffset(c_rot_offset),
      m_cData(un_msg_size),
      m_fRange(f_range) {
      Disable();
      SetCanBeEnabledIfDisabled(false);
      CVector3 cPos = c_pos_offset;
      cPos.Rotate(s_anchor.Orientation);
      cPos += s_anchor.Position;
      SetInitPosition(cPos);
      SetPosition(cPos);
      SetInitOrientation(s_anchor.Orientation * c_rot_offset);
      SetOrientation(GetInitOrientation());
   }

   /****************************************/
   /****************************************/

   void CRABEquippedEntity::Init(TConfigurationNode& t_tree) {
      try {
         /*
          * Init entity.
          * Here we explicitly avoid to call CPositionalEntity::Init() because that
          * would also initialize position and orientation, which, instead, must
          * be calculated from reference entity and offsets.
          */
         CEntity::Init(t_tree);
         /* Get offsets */
         GetNodeAttributeOrDefault(t_tree, "pos_offset", m_cPosOffset, m_cPosOffset);
         std::string strRotOffset;
         GetNodeAttributeOrDefault(t_tree, "rot_offset", strRotOffset, strRotOffset);
         if(strRotOffset != "") {
            CDegrees cRotOffsetEuler[3];
            ParseValues(strRotOffset, 3, cRotOffsetEuler, ',');
            m_cRotOffset.FromEulerAngles(ToRadians(cRotOffsetEuler[0]),
                                         ToRadians(cRotOffsetEuler[1]),
                                         ToRadians(cRotOffsetEuler[2]));
         }
         /* Parse and look up the anchor */
         std::string strAnchorId;
         GetNodeAttribute(t_tree, "anchor", strAnchorId);
         /*
          * NOTE: here we get a reference to the embodied entity
          * This line works under the assumption that:
          * 1. the RABEquippedEntity has a parent;
          * 2. the parent has a child whose id is "body"
          * 3. the "body" is an embodied entity
          * If any of the above is false, this line will bomb out.
          */
         CEmbodiedEntity& cBody = GetParent().GetComponent<CEmbodiedEntity>("body");
         m_psAnchor = &cBody.GetAnchor(strAnchorId);
         /* Set init position and orientation */
         SetInitPosition(m_psAnchor->OffsetPosition + m_cPosOffset); //!?
         SetInitOrientation(m_psAnchor->OffsetOrientation * m_cRotOffset);
         SetPosition(GetInitPosition());
         SetOrientation(GetInitOrientation());
         /* Get message size */
         size_t unMsgSize;
         GetNodeAttribute(t_tree, "msg_size", unMsgSize);
         m_cData.Resize(unMsgSize);
         /* Get transmission range */
         GetNodeAttribute(t_tree, "range", m_fRange);
      }
      catch(CARGoSException& ex) {
         THROW_ARGOSEXCEPTION_NESTED("Error initializing a range and bearing entity \"" << GetId() << "\"", ex);
      }
   }

   /****************************************/
   /****************************************/

   void CRABEquippedEntity::Update() {
      CVector3 cPos = m_cPosOffset;
      cPos.Rotate(m_psAnchor->Orientation);
      cPos += m_psAnchor->Position;
      SetPosition(cPos);
      SetOrientation(m_psAnchor->Orientation * m_cRotOffset);
   }

   /****************************************/
   /****************************************/

   void CRABEquippedEntity::Reset() {
      m_cData.Zero();
   }

   /****************************************/
   /****************************************/

   CEmbodiedEntity& CRABEquippedEntity::GetSensorBody() {
      return GetParent().GetComponent<CEmbodiedEntity>("body"); 
   }
   
   /****************************************/
   /****************************************/

   void CRABEquippedEntity::SetData(const CByteArray& c_data) {
      if(m_cData.Size() == c_data.Size()) {
         m_cData = c_data;
      }
      else {
         THROW_ARGOSEXCEPTION("CRABEquippedEntity::SetData() : data size does not match, expected " << m_cData.Size() << ", got " << c_data.Size());
      }
   }

   /****************************************/
   /****************************************/

   void CRABEquippedEntity::ClearData() {
      m_cData.Zero();
   }

   /****************************************/
   /****************************************/

   void CRABEquippedEntitySpaceHashUpdater::operator()(CAbstractSpaceHash<CRABEquippedEntity>& c_space_hash,
                                                       CRABEquippedEntity& c_element) {
      /* Calculate the position of the center of the RAB equipped entity in the space hash */
      c_space_hash.SpaceToHashTable(m_nCenterI,
                                    m_nCenterJ,
                                    m_nCenterK,
                                    c_element.GetPosition());
      /* Update the cells in a sphere around it */
      SInt32 nRangeI = c_space_hash.SpaceToHashTable(c_element.GetRange(), 0);
      SInt32 nRangeJ;
      SInt32 nRangeK;
      for(SInt32 i = 0; i <= nRangeI; ++i) {
         nRangeJ =
            c_space_hash.SpaceToHashTable(
               ::sqrt(
                  Square(c_element.GetRange()) -
                  Square(c_space_hash.HashTableToSpace(i, 0))
                  ),
               1);
         for(SInt32 j = 0; j <= nRangeJ; ++j) {
            nRangeK =
               c_space_hash.SpaceToHashTable(
                  ::sqrt(
                     Square(c_element.GetRange()) -
                     Square(c_space_hash.HashTableToSpace(j, 1))
                     ),
                  2);
            for(SInt32 k = 0; k <= nRangeK; ++k) {
               if(i > 0) {
                  /*
                   * i > 0
                   */
                  if(j > 0) {
                     /*
                      * i > 0
                      * j > 0
                      */
                     if(k > 0) {
                        /*
                         * i > 0
                         * j > 0
                         * k > 0
                         */
                        c_space_hash.UpdateCell(m_nCenterI + i, m_nCenterJ + j, m_nCenterK + k, c_element);
                        c_space_hash.UpdateCell(m_nCenterI + i, m_nCenterJ + j, m_nCenterK - k, c_element);
                        c_space_hash.UpdateCell(m_nCenterI + i, m_nCenterJ - j, m_nCenterK + k, c_element);
                        c_space_hash.UpdateCell(m_nCenterI + i, m_nCenterJ - j, m_nCenterK - k, c_element);
                        c_space_hash.UpdateCell(m_nCenterI - i, m_nCenterJ + j, m_nCenterK + k, c_element);
                        c_space_hash.UpdateCell(m_nCenterI - i, m_nCenterJ + j, m_nCenterK - k, c_element);
                        c_space_hash.UpdateCell(m_nCenterI - i, m_nCenterJ - j, m_nCenterK + k, c_element);
                        c_space_hash.UpdateCell(m_nCenterI - i, m_nCenterJ - j, m_nCenterK - k, c_element);
                     }
                     else {
                        /*
                         * i > 0
                         * j > 0
                         * k == 0
                         */
                        c_space_hash.UpdateCell(m_nCenterI + i, m_nCenterJ + j, m_nCenterK, c_element);
                        c_space_hash.UpdateCell(m_nCenterI + i, m_nCenterJ - j, m_nCenterK, c_element);
                        c_space_hash.UpdateCell(m_nCenterI - i, m_nCenterJ + j, m_nCenterK, c_element);
                        c_space_hash.UpdateCell(m_nCenterI - i, m_nCenterJ - j, m_nCenterK, c_element);
                     }
                  }
                  else {
                     /*
                      * i > 0
                      * j == 0
                      */
                     if(k > 0) {
                        /*
                         * i > 0
                         * j == 0
                         * k > 0
                         */
                        c_space_hash.UpdateCell(m_nCenterI + i, m_nCenterJ, m_nCenterK + k, c_element);
                        c_space_hash.UpdateCell(m_nCenterI + i, m_nCenterJ, m_nCenterK - k, c_element);
                        c_space_hash.UpdateCell(m_nCenterI - i, m_nCenterJ, m_nCenterK + k, c_element);
                        c_space_hash.UpdateCell(m_nCenterI - i, m_nCenterJ, m_nCenterK - k, c_element);
                     }
                     else {
                        /*
                         * i > 0
                         * j == 0
                         * k == 0
                         */
                        c_space_hash.UpdateCell(m_nCenterI + i, m_nCenterJ, m_nCenterK, c_element);
                        c_space_hash.UpdateCell(m_nCenterI - i, m_nCenterJ, m_nCenterK, c_element);
                     }
                  }
               }
               else {
                  /*
                   * i == 0
                   */
                  if(j > 0) {
                     /*
                      * i == 0
                      * j > 0
                      */
                     if(k > 0) {
                        /*
                         * i == 0
                         * j > 0
                         * k > 0
                         */
                        c_space_hash.UpdateCell(m_nCenterI, m_nCenterJ + j, m_nCenterK + k, c_element);
                        c_space_hash.UpdateCell(m_nCenterI, m_nCenterJ + j, m_nCenterK - k, c_element);
                        c_space_hash.UpdateCell(m_nCenterI, m_nCenterJ - j, m_nCenterK + k, c_element);
                        c_space_hash.UpdateCell(m_nCenterI, m_nCenterJ - j, m_nCenterK - k, c_element);
                     }
                     else {
                        /*
                         * i == 0
                         * j > 0
                         * k == 0
                         */
                        c_space_hash.UpdateCell(m_nCenterI, m_nCenterJ + j, m_nCenterK, c_element);
                        c_space_hash.UpdateCell(m_nCenterI, m_nCenterJ - j, m_nCenterK, c_element);
                     }
                  }
                  else {                     
                     /*
                      * i == 0
                      * j == 0
                      */
                     if(k > 0) {
                        /*
                         * i == 0
                         * j == 0
                         * k > 0
                         */
                        c_space_hash.UpdateCell(m_nCenterI, m_nCenterJ, m_nCenterK + k, c_element);
                        c_space_hash.UpdateCell(m_nCenterI, m_nCenterJ, m_nCenterK - k, c_element);
                     }
                     else {
                        /*
                         * i == 0
                         * j == 0
                         * k == 0
                         */
                        c_space_hash.UpdateCell(m_nCenterI, m_nCenterJ, m_nCenterK, c_element);
                     }
                  }
               }
            }
         }
      }
   }

   /****************************************/
   /****************************************/

   REGISTER_STANDARD_SPACE_OPERATIONS_ON_ENTITY(CRABEquippedEntity);

   /****************************************/
   /****************************************/

   CRABEquippedEntityGridCellUpdater::CRABEquippedEntityGridCellUpdater(CGrid<CRABEquippedEntity>& c_grid) :
      m_cGrid(c_grid) {}
   
   bool CRABEquippedEntityGridCellUpdater::operator()(SInt32 n_i,
                                                      SInt32 n_j,
                                                      SInt32 n_k,
                                                      CGrid<CRABEquippedEntity>::SCell& s_cell) {
      /* Update cell */
      m_cGrid.UpdateCell(n_i, n_j, n_k, *m_pcEntity);
      /* Continue with other cells */
      return true;
   }
   
   void CRABEquippedEntityGridCellUpdater::SetEntity(CRABEquippedEntity& c_entity) {
      m_pcEntity = &c_entity;
   }

   CRABEquippedEntityGridEntityUpdater::CRABEquippedEntityGridEntityUpdater(CGrid<CRABEquippedEntity>& c_grid) :
      m_cGrid(c_grid),
      m_cCellUpdater(c_grid) {}

   bool CRABEquippedEntityGridEntityUpdater::operator()(CRABEquippedEntity& c_entity) {
      try {
         m_cCellUpdater.SetEntity(c_entity);
         m_cGrid.ForCellsInBoxRange(c_entity.GetPosition(),
                                    CVector3(c_entity.GetRange(),
                                             c_entity.GetRange(),
                                             c_entity.GetRange()),
                                    m_cCellUpdater);
         /* Continue with the other entities */
         return true;
      }
      catch(CARGoSException& ex) {
         THROW_ARGOSEXCEPTION_NESTED("While updating the RAB entity grid for RAB entity \"" << c_entity.GetContext() << c_entity.GetId() << "\"", ex);
      }
   }

   /****************************************/
   /****************************************/

}
