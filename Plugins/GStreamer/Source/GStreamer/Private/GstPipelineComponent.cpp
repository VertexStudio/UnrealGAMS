#include "GstPipelineComponent.h"
#include "GstAppSinkComponent.h"
#include "GstAppSrcComponent.h"
#include "GameFramework/Actor.h"

UGstPipelineComponent::UGstPipelineComponent()
{
}

void UGstPipelineComponent::UninitializeComponent()
{
	ResetState();
}

void UGstPipelineComponent::ResetState()
{
	SafeDestroy(Pipeline);
}

void UGstPipelineComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PipelineAutostart)
	{
		StartPipeline();
	}
}

void UGstPipelineComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	StopPipeline();
}

bool UGstPipelineComponent::StartPipeline()
{
	if (Pipeline)
	{
		GST_LOG_ERR(TEXT("GstPipelineComponent: Already started"));
		return false;
	}

	if (!PipelineConfig.IsEmpty())
	{
		if (UseGstMainLoop)
		{
			Pipeline = IGstPipeline::CreateLoop();
		}
		else
		{
			Pipeline = IGstPipeline::CreateTick();
		}
		if (Pipeline->Init(TCHAR_TO_ANSI(*PipelineName), TCHAR_TO_ANSI(*PipelineConfig)))
		{
			{
				TInlineComponentArray<UGstAppSinkComponent *> Components;
				GetOwner()->GetComponents(Components);
				for (auto *Comp : Components)
				{
					if (Comp->PipelineName == PipelineName)
					{
						Comp->CbPipelineStart(Pipeline);
					}
				}
			}
			{
				TInlineComponentArray<UGstAppSrcComponent *> Components;
				GetOwner()->GetComponents(Components);
				for (auto *Comp : Components)
				{
					if (Comp->PipelineName == PipelineName)
					{
						Comp->CbPipelineStart(Pipeline);
					}
				}
			}

			if (Pipeline->Start())
				return true;
		}
	}

	StopPipeline();
	return false;
}

void UGstPipelineComponent::StopPipeline()
{
	if (Pipeline)
	{
		{
			TInlineComponentArray<UGstAppSinkComponent *> Components;
			GetOwner()->GetComponents(Components);
			for (auto *Comp : Components)
			{
				if (Comp->PipelineName == PipelineName)
				{
					Comp->CbPipelineStop();
				}
			}
		}
		{
			TInlineComponentArray<UGstAppSrcComponent *> Components;
			GetOwner()->GetComponents(Components);
			for (auto *Comp : Components)
			{
				if (Comp->PipelineName == PipelineName)
				{
					Comp->CbPipelineStop();
				}
			}
		}
	}

	ResetState();
}
