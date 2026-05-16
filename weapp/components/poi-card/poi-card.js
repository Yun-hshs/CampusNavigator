Component({
  properties: {
    id: { type: Number, value: 0 },
    name: { type: String, value: '' },
    type: { type: String, value: '' },
    desc: { type: String, value: '' },
    latitude: { type: Number, value: 0 },
    longitude: { type: Number, value: 0 }
  },

  methods: {
    onTap() {
      this.triggerEvent('tap', {
        id: this.data.id,
        name: this.data.name,
        latitude: this.data.latitude,
        longitude: this.data.longitude
      });
    }
  }
});