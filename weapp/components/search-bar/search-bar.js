Component({
  properties: {
    placeholder: { type: String, value: '搜索地点' },
    value: { type: String, value: '' },
    focus: { type: Boolean, value: false },
    showState: { type: Boolean, value: false },
    searching: { type: Boolean, value: false },
    empty: { type: Boolean, value: false }
  },

  methods: {
    onInput(e) {
      this.triggerEvent('input', { value: e.detail.value });
    },
    onClear() {
      this.triggerEvent('clear');
    },
    onConfirm(e) {
      this.triggerEvent('confirm', { value: e.detail.value });
    }
  }
});
