// pages/params/params.js
import { bleManager } from '../../utils/ble.js'

const app = getApp()

Page({
  data: {
    connected: false,
    saving: false,
    resetting: false,
    // 天气配置
    weatherTypes: ['实时天气', '每日预报'],
    weatherTypeIndex: 0,
    city: '',
    apiKey: '',
    weekInfoOptions: ['显示', '隐藏'],
    weekInfoIndex: 0,
    showGold: true,
    // 标签配置
    tagDays: 7,
    examDate: '',
    dayLabel: '',
    // 日程配置
    scheduleOptions: ['显示', '隐藏'],
    scheduleIndex: 0,
    scheduleReminder: true
  },

  onLoad() {
    this.checkConnection()
    this.loadConfig()
  },

  onShow() {
    this.checkConnection()
  },

  checkConnection() {
    this.setData({ connected: app.globalData.connected })
  },

  // 加载保存的配置
  loadConfig() {
    const config = wx.getStorageSync('params_config')
    if (config) {
      this.setData({
        weatherTypeIndex: config.weatherTypeIndex || 0,
        city: config.city || '',
        apiKey: config.apiKey || '',
        weekInfoIndex: config.weekInfoIndex || 0,
        showGold: config.showGold !== undefined ? config.showGold : true,
        tagDays: config.tagDays || 7,
        examDate: config.examDate || '',
        dayLabel: config.dayLabel || '',
        scheduleIndex: config.scheduleIndex || 0,
        scheduleReminder: config.scheduleReminder !== undefined ? config.scheduleReminder : true
      })
    }
  },

  onWeatherTypeChange(e) {
    this.setData({ weatherTypeIndex: parseInt(e.detail.value) })
  },

  onCityInput(e) {
    this.setData({ city: e.detail.value })
  },

  onApiKeyInput(e) {
    this.setData({ apiKey: e.detail.value })
  },

  onWeekInfoChange(e) {
    this.setData({ weekInfoIndex: parseInt(e.detail.value) })
  },

  onGoldChange(e) {
    this.setData({ showGold: e.detail.value })
  },

  onTagDaysInput(e) {
    this.setData({ tagDays: parseInt(e.detail.value) || 7 })
  },

  onExamDateChange(e) {
    this.setData({ examDate: e.detail.value })
  },

  onDayLabelInput(e) {
    this.setData({ dayLabel: e.detail.value })
  },

  onScheduleChange(e) {
    this.setData({ scheduleIndex: parseInt(e.detail.value) })
  },

  onReminderChange(e) {
    this.setData({ scheduleReminder: e.detail.value })
  },

  // 保存配置
  async saveConfig() {
    this.setData({ saving: true })

    const config = {
      cmd: 'set_params',
      data: {
        weather_type: this.data.weatherTypeIndex === 0 ? 1 : 2,
        default_city: this.data.city,
        api_key: this.data.apiKey,
        week_info: this.data.weekInfoIndex === 0 ? 'on' : 'off',
        widgets_gold: this.data.showGold ? 'on' : 'off',
        tag_days: this.data.tagDays,
        cd_day_label: this.data.dayLabel,
        cd_day_date: this.data.examDate,
        study_schedule: this.data.scheduleIndex === 0 ? 'on' : 'off'
      }
    }

    try {
      await bleManager.sendData(JSON.stringify(config))

      bleManager.onDataReceived((data) => {
        try {
          const response = JSON.parse(data)
          if (response.cmd === 'set_params_result') {
            this.setData({ saving: false })
            
            if (response.data.success) {
              // 保存到本地
              wx.setStorageSync('params_config', {
                weatherTypeIndex: this.data.weatherTypeIndex,
                city: this.data.city,
                apiKey: this.data.apiKey,
                weekInfoIndex: this.data.weekInfoIndex,
                showGold: this.data.showGold,
                tagDays: this.data.tagDays,
                examDate: this.data.examDate,
                dayLabel: this.data.dayLabel,
                scheduleIndex: this.data.scheduleIndex,
                scheduleReminder: this.data.scheduleReminder
              })
              
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
  },

  // 恢复默认
  async resetConfig() {
    wx.showModal({
      title: '确认重置',
      content: '确定要恢复默认配置吗？',
      success: async (res) => {
        if (res.confirm) {
          this.setData({ resetting: true })
          
          try {
            await bleManager.sendData(JSON.stringify({
              cmd: 'reset_params',
              data: {}
            }))

            bleManager.onDataReceived((data) => {
              try {
                const response = JSON.parse(data)
                if (response.cmd === 'reset_result') {
                  this.setData({ resetting: false })
                  
                  wx.showToast({
                    title: '重置成功',
                    icon: 'success'
                  })

                  // 重新加载配置
                  setTimeout(() => {
                    this.loadConfig()
                  }, 2000)
                }
              } catch (error) {
                console.error('解析响应失败:', error)
                this.setData({ resetting: false })
              }
            })

            setTimeout(() => {
              if (this.data.resetting) {
                this.setData({ resetting: false })
              }
            }, 10000)
          } catch (error) {
            console.error('重置失败:', error)
            wx.showToast({
              title: '操作失败',
              icon: 'none'
            })
            this.setData({ resetting: false })
          }
        }
      }
    })
  }
})
