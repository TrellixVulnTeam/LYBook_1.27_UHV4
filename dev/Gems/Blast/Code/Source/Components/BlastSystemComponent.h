/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
 * its licensors.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */
#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <Blast/BlastDebug.h>
#include <Blast/BlastSystemBus.h>
#include <CrySystemBus.h>
#include <NvBlastExtDamageShaders.h>
#include <NvBlastExtPxTask.h>
#include <NvBlastExtSerialization.h>
#include <NvBlastGlobals.h>
#include <NvBlastProfiler.h>
#include <NvBlastTkFramework.h>
#include <NvBlastTkGroup.h>
#include <PxSmartPtr.h>
#include <Task/PxTaskManager.h>

namespace Blast
{
    class BlastSystemComponent
        : public AZ::Component
        , BlastSystemRequestBus::Handler
        , public AZ::Interface<Blast::BlastSystemRequests>::Registrar
        , public AZ::TickBus::Handler
        , private CrySystemEventBus::Handler
        , private AZ::Data::AssetBus::MultiHandler
    {
    public:
        AZ_COMPONENT(BlastSystemComponent, "{9705144A-FF10-45CE-AA3D-3E1F43872429}");
        BlastSystemComponent() = default;
        BlastSystemComponent(const BlastSystemComponent&) = delete;
        BlastSystemComponent& operator=(const BlastSystemComponent&) = delete;

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        // AZTickBus interface implementation
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        // AZ::Data::AssetBus interface implementation
        void OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;

        // Blast configuration methods
        void LoadConfiguration();
        void SaveConfiguration();
        void CheckoutConfiguration();

        // CrySystemEventBus interface implementation
        void OnCrySystemInitialized(ISystem&, const SSystemInitParams&) override;
        void OnCryEditorInitialized() override;

        // Getters for physx/NvBlast structures
        Nv::Blast::TkFramework* GetTkFramework() const override;
        Nv::Blast::ExtSerialization* GetExtSerialization() const override;
        Nv::Blast::TkGroup* GetTkGroup() override;

        void AddDamageDesc(AZStd::unique_ptr<NvBlastExtRadialDamageDesc> desc) override;
        void AddDamageDesc(AZStd::unique_ptr<NvBlastExtCapsuleRadialDamageDesc> desc) override;
        void AddDamageDesc(AZStd::unique_ptr<NvBlastExtShearDamageDesc> desc) override;
        void AddDamageDesc(AZStd::unique_ptr<NvBlastExtTriangleIntersectionDamageDesc> desc) override;
        void AddDamageDesc(AZStd::unique_ptr<NvBlastExtImpactSpreadDamageDesc> desc) override;
        void AddProgramParams(AZStd::unique_ptr<NvBlastExtProgramParams> program) override;

        const BlastGlobalConfiguration& GetGlobalConfiguration() const override;
        void SetGlobalConfiguration(const BlastGlobalConfiguration& materialLibrary) override;
        void SetDebugRenderMode(DebugRenderMode debugRenderMode) override;

    protected:
        void InitPhysics();
        void DeactivatePhysics();

        void RegisterCommands();

        // Internal helper functions & classes
        class AZBlastAllocatorCallback : public Nv::Blast::AllocatorCallback
        {
        public:
            void* allocate(
                size_t size, const char* typeName, [[maybe_unused]] const char* filename,
                [[maybe_unused]] int line) override
            {
                return azmalloc_4(size, 0, AZ::SystemAllocator, typeName);
            }

            void deallocate(void* ptr) override
            {
                azfree(ptr);
            }
        };

        class AZBlastProfilerCallback : public Nv::Blast::ProfilerCallback
        {
        public:
            void zoneStart(const char* eventName) override;
            void zoneEnd() override;
        };

        struct BlastGroup
        {
            physx::unique_ptr<Nv::Blast::TkGroup> m_tkGroup;
            physx::unique_ptr<Nv::Blast::ExtGroupTaskManager> m_extGroupTaskManager;
        };

        AZBlastAllocatorCallback m_blastAllocatorCallback;
        AZBlastProfilerCallback m_blastProfilerCallback;

        AZStd::vector<BlastGroup> m_groups;

        // Container for asset types that need to be registered
        AZStd::vector<AZStd::unique_ptr<AZ::Data::AssetHandler>> m_assetHandlers;

        // Blast framework & physics singletons, in order of initialization
        physx::unique_ptr<Nv::Blast::TkFramework> m_tkFramework;
        physx::unique_ptr<Nv::Blast::ExtSerialization> m_extSerialization;
        physx::unique_ptr<physx::PxTaskManager> m_defaultTaskManager;

        // Library for blast materials and other global configurations
        BlastGlobalConfiguration m_configuration;

        // Storage for damage info that gets simulated.
        AZStd::vector<AZStd::unique_ptr<NvBlastExtRadialDamageDesc>> m_radialDamageDescs;
        AZStd::vector<AZStd::unique_ptr<NvBlastExtCapsuleRadialDamageDesc>> m_capsuleDamageDescs;
        AZStd::vector<AZStd::unique_ptr<NvBlastExtShearDamageDesc>> m_shearDamageDescs;
        AZStd::vector<AZStd::unique_ptr<NvBlastExtTriangleIntersectionDamageDesc>> m_triangleDamageDescs;
        AZStd::vector<AZStd::unique_ptr<NvBlastExtImpactSpreadDamageDesc>> m_impactDamageDescs;
        AZStd::vector<AZStd::unique_ptr<NvBlastExtProgramParams>> m_programParams;

        bool m_registered;
        DebugRenderMode m_debugRenderMode;
    };
} // namespace Blast
