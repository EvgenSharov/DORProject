#include "PrjImporter.h"
#include "Core.h"
#include "ModuleManager.h"
//#include "AssetToolsModule.h"

IMPLEMENT_MODULE(FPrjImporterModule, PrjImporter);

DEFINE_LOG_CATEGORY(PrjImporter)

void FPrjImporterModule::StartupModule()
{
	UE_LOG(PrjImporter, Warning, TEXT("PrjImporter: Log Started"));
}

void FPrjImporterModule::ShutdownModule()
{
	UE_LOG(PrjImporter, Warning, TEXT("PrjImporter: Log Ended"));
}
