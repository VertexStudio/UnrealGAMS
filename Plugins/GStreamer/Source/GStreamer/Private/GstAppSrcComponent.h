#pragma once

#include "GstElementComponent.h"
#include "GstAppSrcImpl.h"
#include "GstVideoFormat.h"
#include "GstAppSrcComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GSTREAMER_API UGstAppSrcComponent : public UGstElementComponent
{
	GENERATED_BODY()

  public:
	UGstAppSrcComponent();

	virtual void UninitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void CbPipelineStart(class IGstPipeline *Pipeline);
	virtual void CbPipelineStop();

	UPROPERTY(Category = "GstAppSrc", EditAnywhere, BlueprintReadWrite)
	FString PipelineName;

	UPROPERTY(Category = "GstAppSrc", EditAnywhere, BlueprintReadWrite)
	FString AppSrcName;

	UPROPERTY(Category = "GstAppSrc", EditAnywhere, BlueprintReadWrite)
	bool AppSrcEnabled;

	UPROPERTY(Category = "GstAppSrc", EditAnywhere)
	TArray<FComponentReference> AppSrcCaptures;

  protected:
	void ResetState();

	IGstAppSrc *AppSrc = nullptr;
};
