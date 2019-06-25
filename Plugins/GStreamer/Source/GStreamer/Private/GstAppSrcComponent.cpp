#include "GstAppSrcComponent.h"
#include "GstPipelineImpl.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/Core/Public/Async/Async.h"

UGstAppSrcComponent::UGstAppSrcComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1;
}

void UGstAppSrcComponent::UninitializeComponent()
{
	ResetState();
}

void UGstAppSrcComponent::ResetState()
{
	if (AppSrc)
		AppSrc->Disconnect();
	SafeDestroy(AppSrc);
}

void UGstAppSrcComponent::CbPipelineStart(IGstPipeline *Pipeline)
{
	ResetState();

	if (AppSrcEnabled && !AppSrcName.IsEmpty())
	{
		AppSrc = IGstAppSrc::CreateInstance();
		AppSrc->Connect(Pipeline, TCHAR_TO_ANSI(*AppSrcName));
	}
}

void UGstAppSrcComponent::CbPipelineStop()
{
	ResetState();
}

void UGstAppSrcComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (AppSrc)
	{
		AActor *Actor = GetOwner();
		for (FComponentReference ComponentReference : AppSrcCaptures)
		{
			USceneCaptureComponent2D *CaptureComponent = Cast<USceneCaptureComponent2D>(ComponentReference.GetComponent(Actor));
			UTextureRenderTarget2D *TextureTarget = CaptureComponent->TextureTarget;
			TArray<FColor> TextureData;
			FTextureRenderTargetResource *TextureResource = TextureTarget->GameThread_GetRenderTargetResource();
			TextureResource->ReadPixels(TextureData);
			AppSrc->PushTexture((uint8_t *)TextureData.GetData(), TextureData.Num() * 4);
		}
	}
}
