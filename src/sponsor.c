#include "sponsor.h"

#include "log.h"
#include "service.h"

/*
 * Prints the sponsor of a model configuration.
 *
 * The following fields are guaranteed to exist:
 *   - sponsor->Name
 *   - sponsor->URL
 *
 * The following fields are optional:
 *   - sponsor->Description
 */
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
