export function singleTouch(target) {
  const secondary = new Set();
  target.addEventListener('pointerdown', event => {
    secondary.delete(event.pointerId);
    if (event.pointerType === 'touch' && !event.isPrimary) {
      secondary.add(event.pointerId);
      event.preventDefault();
      event.stopImmediatePropagation();
    }
  }, true);
  // click.isPrimary defaults to false even for a primary touch click.
  target.addEventListener('click', event => {
    if (event.pointerType === 'touch' && secondary.delete(event.pointerId)) {
      event.preventDefault();
      event.stopImmediatePropagation();
    }
  }, true);
  target.addEventListener('pointercancel', event => secondary.delete(event.pointerId), true);
}
