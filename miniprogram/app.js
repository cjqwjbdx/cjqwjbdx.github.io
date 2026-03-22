// app.js
App({
  onLaunch() {
    console.log('App Launch')
  },
  onShow() {
    console.log('App Show')
  },
  onHide() {
    console.log('App Hide')
  },
  onError(msg) {
    console.log('App Error:', msg)
  },
  globalData: {
    userInfo: null,
    deviceId: null,
    serviceId: null,
    characteristicId: null,
    connected: false
  }
})
