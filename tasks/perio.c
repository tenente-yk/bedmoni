/*! \file perio.c
    \brief Periphery i/o.
*/

#include "bedmoni.h"
#include "crbpio.h"
#include "csio.h"
#include "perio.h"

int perio_init(void)
{
  crbpio_init();

  csio_init();

  return 0;
}

void perio_deinit(void)
{
  csio_deinit();

  crbpio_deinit();
}

void perio_update(void)
{
  crbpio_update();

  csio_update();
}
