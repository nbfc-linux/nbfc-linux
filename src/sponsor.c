#include "sponsor.h"

#include "log.h"
#include "service.h"

void Sponsor_Print() {
  if (! ModelConfig_IsSet_Sponsor(&Service_ModelConfig))
    return;

  const Sponsor* sponsor = &Service_ModelConfig.Sponsor;

  Log_Info("This configuration is sponsored by:");

  if (Sponsor_IsSet_Description(sponsor))
    Log_Info("%s - %s (%s)", sponsor->Name, sponsor->Description, sponsor->URL);
  else
    Log_Info("%s (%s)", sponsor->Name, sponsor->URL);
}
