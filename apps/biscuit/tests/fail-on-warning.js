// Preloaded into every `node --test` run (see the Makefile): any process warning (deprecation,
// experimental, ...) fails the test file that raised it. Warnings are errors.
process.on('warning', () => {
  process.exitCode = 1;
});
