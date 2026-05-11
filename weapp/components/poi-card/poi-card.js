Component({
  properties: {
    id: { type: Number, value: 0 },
    name: { type: String, value: '' },
    type: { type: String, value: '' },
    desc: { type: String, value: '' }
  },

  methods: {
    onTap() {
      this.triggerEvent('tap', {
        id: this.data.id,
        name: this.data.name
      });
    }
  }
});
