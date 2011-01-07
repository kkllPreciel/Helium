#pragma once

#include "Pipeline/API.h"
#include "Pipeline/Settings.h"
#include "Pipeline/SceneGraph/Manipulator.h"
#include "Pipeline/SceneGraph/TranslateManipulator.h"

namespace Helium
{
    namespace SceneGraph
    {
        class PIPELINE_API SceneSettings : public Settings
        {
        public:
            REFLECT_DECLARE_CLASS( SceneSettings, Settings );

            SceneSettings();

            float32_t ScaleManipulatorSize() const;
            bool ScaleManipulatorGridSnap() const;
            float32_t ScaleManipulatorDistance() const;

            float32_t RotateManipulatorSize() const;
            bool RotateManipulatorAxisSnap() const;
            float32_t RotateManipulatorSnapDegrees() const;
            ManipulatorSpace RotateManipulatorSpace() const;

            float32_t TranslateManipulatorSize() const;
            ManipulatorSpace TranslateManipulatorSpace() const;
            TranslateSnappingMode TranslateManipulatorSnappingMode() const;
            float32_t TranslateManipulatorDistance() const;
            bool TranslateManipulatorLiveObjectsOnly() const;

        private:
            float32_t m_ScaleManipulatorSize;
            bool m_ScaleManipulatorGridSnap;
            float32_t m_ScaleManipulatorDistance;

            float32_t m_RotateManipulatorSize;
            bool m_RotateManipulatorAxisSnap;
            float32_t m_RotateManipulatorSnapDegrees;
            ManipulatorSpace m_RotateManipulatorSpace;

            float32_t m_TranslateManipulatorSize;
            ManipulatorSpace m_TranslateManipulatorSpace;
            TranslateSnappingMode m_TranslateManipulatorSnappingMode;
            float32_t m_TranslateManipulatorDistance;
            bool m_TranslateManipulatorLiveObjectsOnly;

        public:
            static void AcceptCompositeVisitor( Reflect::Composite& comp )
            {
                comp.AddField( &SceneSettings::m_ScaleManipulatorSize, TXT( "Scale Manipulator Size" ) );
                comp.AddField( &SceneSettings::m_ScaleManipulatorGridSnap, TXT( "Scale Manipulator Grid Snapping" ) );
                comp.AddField( &SceneSettings::m_ScaleManipulatorDistance, TXT( "Scale Manipulator Distance" ) );

                comp.AddField( &SceneSettings::m_RotateManipulatorSize, TXT( "Rotate Manipulator Size" ) );
                comp.AddField( &SceneSettings::m_RotateManipulatorAxisSnap, TXT( "Rotate Manipulator Axis Snapping" ) );
                comp.AddField( &SceneSettings::m_RotateManipulatorSnapDegrees, TXT( "Rotate Manipulator Snap Degrees" ) );
                comp.AddEnumerationField( &SceneSettings::m_RotateManipulatorSpace, TXT( "Rotate Manipulator Space" ) );

                comp.AddField( &SceneSettings::m_TranslateManipulatorSize, TXT( "Translate Manipulator Size" ) );
                comp.AddEnumerationField( &SceneSettings::m_TranslateManipulatorSpace, TXT( "Translate Manipulator Space" ) );
                comp.AddEnumerationField( &SceneSettings::m_TranslateManipulatorSnappingMode, TXT( "Translate Manipulator Snapping Mode" ) );
                comp.AddField( &SceneSettings::m_TranslateManipulatorDistance, TXT( "Translate Manipulator Distance" ) );
                comp.AddField( &SceneSettings::m_TranslateManipulatorLiveObjectsOnly, TXT( "Translate Manipulator Live Objects Only" ) );
            }
        };
        typedef Helium::SmartPtr< SceneSettings > SceneSettingsPtr;
    }
}