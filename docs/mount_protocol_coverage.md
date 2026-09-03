# ASDEVLAB OnStepX LX200 Coverage

This coverage report maps the OnStepX LX200 firmware command surface to ASDEVLAB wrapper, parser, and test support.

## Coverage summary

- Total commands: 162
- Wrapper support: 33 (20.4%)
- Parser support: 17 (10.5%)
- Test coverage: 31 (19.1%)

## Strengths

- Core motion and state commands are well supported.
- Status, RA/Dec, go-to/sync, site/time, limits, PEC, focuser, and flip mirror flows are covered.
- Adapter test coverage confirms command emission for many key control paths.

## Known gaps

- Advanced :GX... and :SX... configuration/diagnostic commands are not broadly surfaced.
- Library and ST4 command families are not wrapped.
- Some set-only commands and advanced tracking/compensation controls remain unsupported.
