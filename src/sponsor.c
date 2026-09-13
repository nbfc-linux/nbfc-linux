#include "sponsor.h"

#include "log.h"
#include "service.h"

void Sponsor_Print(void) {
  if (! Service_ModelConfig.isset.Sponsor)
    return;

  const Sponsor* sponsor = &Service_ModelConfig.Sponsor;

  Log_Info("This configuration is sponsored by:");

  if (sponsor->isset.Description)
    Log_Info("%s - %s (%s)", sponsor->Name, sponsor->Description, sponsor->URL);
  else
    Log_Info("%s (%s)", sponsor->Name, sponsor->URL);
}
