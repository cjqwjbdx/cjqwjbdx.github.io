// pages/wifi/wifi.js
import { bleManager } from '../../utils/ble.js'

const app = getApp()

Page({
  data: {
    connected: false,
    ssid: '',
    password: '',
    showPassword: false,
    saving: false,
    scanning: false,
    wifiList: []
  },

  onLoad() {
    this.checkConnection()
    
    // 加载保存的配置
    const savedSSID = wx.getStorageSync('wifi_ssid')
    const savedPassword = wx.getStorageSync('wifi_password')
    if (savedSSID) {
      this.setData({ ssid: savedSSID })
    }
  },

  onShow() {
    this.checkConnection()
  },

  checkConnection() {
    this.setData({ connected: app.globalData.connected })
  },

  onSSIDInput(e) {
    this.setData({ ssid: e.detail.value })
  },

  onPasswordInput(e) {
    this.setData({ password: e.detail.value })
  },

  togglePassword() {
    this.setData({ showPassword: !this.data.showPassword })
  },

  // 扫描WiFi
  async scanWifi() {
    this.setData({ scanning: true })
    
    try {
      // 发送扫描命令
      await bleManager.sendData(JSON.stringify({
        cmd: 'scan_wifi',
        data: {}
      }))

      // 监听响应
      bleManager.onDataReceived((data) => {
        try {
          const response = JSON.parse(data)
          if (response.cmd === 'wifi_scan_result') {
            const wifiList = response.data.list.map(item => ({
              ssid: item.ssid,
              signal: item.rssi,
              secured: item.auth
            })).sort((a, b) => b.signal - a.signal)
            
            this.setData({ wifiList, scanning: false })
          }
        } catch (error) {
          console.error('解析WiFi列表失败:', error)
          this.setData({ scanning: false })
        }
      })

      // 10秒超时
      setTimeout(() => {
        if (this.data.scanning) {
          this.setData({ scanning: false })
        }
      }, 10000)
    } catch (error) {
      console.error('扫描失败:', error)
      wx.showToast({
        title: '扫描失败',
        icon: 'none'
      })
      this.setData({ scanning: false })
    }
  },

  selectWifi(e) {
    const ssid = e.currentTarget.dataset.ssid
    this.setData({ ssid })
  },

  // 保存配置
  async saveConfig() {
    if (!this.data.ssid || !this.data.password) {
      wx.showToast({
        title: '请填写完整信息',
        icon: 'none'
      })
      return
    }

    this.setData({ saving: true })

    try {
      // 发送WiFi配置
      await bleManager.sendData(JSON.stringify({
        cmd: 'set_wifi',
        data: {
          ssid: this.data.ssid,
          password: this.data.password
        }
      }))

      // 监听响应
      bleManager.onDataReceived((data) => {
        try {
          const response = JSON.parse(data)
          if (response.cmd === 'set_wifi_result') {
            this.setData({ saving: false })
            
            if (response.data.success) {
              // 保存到本地
              wx.setStorageSync('wifi_ssid', this.data.ssid)
              wx.setStorageSync('wifi_password', this.data.password)
              
              wx.showToast({
                title: '保存成功',
                icon: 'success'
              })
            } else {
              wx.showToast({
                title: '保存失败',
                icon: 'none'
              })
            }
          }
        } catch (error) {
          console.error('解析响应失败:', error)
          this.setData({ saving: false })
        }
      })

      // 10秒超时
      setTimeout(() => {
        if (this.data.saving) {
          this.setData({ saving: false })
          wx.showToast({
            title: '操作超时',
            icon: 'none'
          })
        }
      }, 10000)
    } catch (error) {
      console.error('保存失败:', error)
      wx.showToast({
        title: '发送失败',
        icon: 'none'
      })
      this.setData({ saving: false })
    }
  }
})
