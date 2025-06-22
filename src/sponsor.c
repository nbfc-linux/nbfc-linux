#include "sponsor.h"

#include "log.h"
#include "service.h"

void Sponsor_Print() {
  if (! ModelConfig_IsSet_Sponsor(&Service_Model_Config))
    return;

  const Sponsor* sponsor = &Service_Model_Config.Sponsor;

  Log_Info("This configuration is sponsored by:\n");

  if (Sponsor_IsSet_Description(sponsor))
    Log_Info("%s - %s (%s)\n", sponsor->Name, sponsor->Description, sponsor->URL);
  else
    Log_Info("%s (%s)\n", sponsor->Name, sponsor->URL);
}
