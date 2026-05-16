Component({
  properties: {
    distance: { type: Number, value: 0 },
    duration: { type: Number, value: 0 },
    steps: { type: Array, value: [] },
    mode: { type: String, value: 'walk' }
  },

  data: {
    displayDistance: '',
    displayTime: ''
  },

  observers: {
    'distance, duration, mode': function(dist, dur, mode) {
      let dStr = dist >= 1000 ? (dist / 1000).toFixed(1) + 'km' : Math.round(dist) + 'm';
      let tStr = '';
      if (dur < 1) tStr = '不到1分钟';
      else if (dur >= 60) {
        const h = Math.floor(dur / 60);
        const m = Math.round(dur % 60);
        tStr = m > 0 ? h + '小时' + m + '分钟' : h + '小时';
      } else {
        tStr = '约' + Math.ceil(dur) + '分钟';
      }
      this.setData({ displayDistance: dStr, displayTime: tStr });
    }
  }
});
