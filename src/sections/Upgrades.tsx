import { useEffect, useRef } from 'react';
import { gsap } from 'gsap';
import { ScrollTrigger } from 'gsap/ScrollTrigger';
import { CloudSun, Database, Thermometer, Battery, Bluetooth } from 'lucide-react';

gsap.registerPlugin(ScrollTrigger);

interface Upgrade {
  icon: React.ElementType;
  title: string;
  description: string;
}

const upgrades1Plus: Upgrade[] = [
  {
    icon: CloudSun,
    title: '天气预报',
    description: '预知未来天气，提前规划行程',
  },
  {
    icon: Database,
    title: '内存升级',
    description: '从 4MB 升级至 8MB，运行更流畅',
  },
];

const upgrades1Pro: Upgrade[] = [
  {
    icon: Thermometer,
    title: '温湿度传感器',
    description: '实时监测房间温度和湿度',
  },
  {
    icon: Battery,
    title: '超低功耗设计',
    description: '全新电源管理，续航更持久',
  },
  {
    icon: Bluetooth,
    title: '蓝牙小程序配置',
    description: '手机一键配置，轻松上手',
  },
];

const Upgrades = () => {
  const sectionRef = useRef<HTMLElement>(null);
  const plusRef = useRef<HTMLDivElement>(null);
  const proRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const section = sectionRef.current;
    const plus = plusRef.current;
    const pro = proRef.current;

    if (!section || !plus || !pro) return;

    const ctx = gsap.context(() => {
      // 1+ Section Animation
      const plusImage = plus.querySelector('.product-image-container');
      const plusContent = plus.querySelector('.content-container');

      gsap.fromTo(
        plusImage,
        { clipPath: 'inset(0 100% 0 0)' },
        {
          clipPath: 'inset(0 0% 0 0)',
          duration: 1,
          ease: 'expo.out',
          scrollTrigger: {
            trigger: plus,
            start: 'top 75%',
            toggleActions: 'play none none reverse',
          },
        }
      );

      const plusTitle = plusContent?.querySelector('h3');
      if (plusTitle) {
        gsap.fromTo(
          plusTitle,
          { opacity: 0, y: 40 },
          {
            opacity: 1,
            y: 0,
            duration: 0.6,
            ease: 'expo.out',
            scrollTrigger: {
              trigger: plus,
              start: 'top 70%',
              toggleActions: 'play none none reverse',
            },
            delay: 0.3,
          }
        );
      }

      const plusPrice = plusContent?.querySelector('.price');
      if (plusPrice) {
        gsap.fromTo(
          plusPrice,
          { opacity: 0, scale: 0.5 },
          {
            opacity: 1,
            scale: 1,
            duration: 0.4,
            ease: 'back.out(1.7)',
            scrollTrigger: {
              trigger: plus,
              start: 'top 70%',
              toggleActions: 'play none none reverse',
            },
            delay: 0.5,
          }
        );
      }

      const plusItems = plus.querySelectorAll('.upgrade-item');
      plusItems.forEach((item, index) => {
        gsap.fromTo(
          item,
          { opacity: 0, y: 30 },
          {
            opacity: 1,
            y: 0,
            duration: 0.4,
            ease: 'expo.out',
            scrollTrigger: {
              trigger: plus,
              start: 'top 65%',
              toggleActions: 'play none none reverse',
            },
            delay: 0.6 + index * 0.1,
          }
        );
      });

      // 1Pro Section Animation
      const proImage = pro.querySelector('.product-image-container');
      const proContent = pro.querySelector('.content-container');

      gsap.fromTo(
        proContent,
        { opacity: 0, x: -50 },
        {
          opacity: 1,
          x: 0,
          duration: 0.6,
          ease: 'expo.out',
          scrollTrigger: {
            trigger: pro,
            start: 'top 75%',
            toggleActions: 'play none none reverse',
          },
        }
      );

      gsap.fromTo(
        proImage,
        { clipPath: 'inset(0 0 0 100%)' },
        {
          clipPath: 'inset(0 0 0 0%)',
          duration: 1,
          ease: 'expo.out',
          scrollTrigger: {
            trigger: pro,
            start: 'top 75%',
            toggleActions: 'play none none reverse',
          },
          delay: 0.2,
        }
      );

      const proItems = pro.querySelectorAll('.upgrade-item');
      proItems.forEach((item, index) => {
        gsap.fromTo(
          item,
          { opacity: 0, x: -20 },
          {
            opacity: 1,
            x: 0,
            duration: 0.4,
            ease: 'expo.out',
            scrollTrigger: {
              trigger: pro,
              start: 'top 65%',
              toggleActions: 'play none none reverse',
            },
            delay: 0.4 + index * 0.1,
          }
        );
      });

      // Parallax effects
      gsap.to([plusImage, proImage], {
        y: -40,
        ease: 'none',
        scrollTrigger: {
          trigger: section,
          start: 'top bottom',
          end: 'bottom top',
          scrub: 0.5,
        },
      });
    }, section);

    return () => ctx.revert();
  }, []);

  return (
    <section
      id="upgrades"
      ref={sectionRef}
      className="relative py-24 md:py-32 bg-white overflow-hidden"
    >
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        {/* Section Title */}
        <div className="text-center mb-16">
          <h2 className="heading-section text-[var(--apple-dark)] mb-4">
            全面升级
          </h2>
          <p className="body-large">更多功能，更好体验</p>
        </div>

        {/* 1+ Section */}
        <div
          ref={plusRef}
          className="grid lg:grid-cols-5 gap-8 lg:gap-12 items-center mb-24"
        >
          {/* Image */}
          <div className="lg:col-span-3 product-image-container">
            <div className="relative overflow-hidden rounded-2xl">
              <img
                src="/images/product-white.png"
                alt="WuJin Calendar 1+"
                className="w-full h-auto"
              />
              <div className="absolute inset-0 bg-gradient-to-t from-black/20 to-transparent" />
            </div>
          </div>

          {/* Content */}
          <div className="lg:col-span-2 content-container">
            <h3 className="heading-subsection text-[var(--apple-dark)] mb-2">
              WuJin Calendar 1+
            </h3>
            <p className="price text-3xl font-bold text-[var(--apple-blue)] mb-6">
              ¥97.99
            </p>

            <div className="space-y-4">
              {upgrades1Plus.map((upgrade) => {
                const Icon = upgrade.icon;
                return (
                  <div
                    key={upgrade.title}
                    className="upgrade-item flex items-start gap-4 p-4 rounded-xl bg-[var(--apple-light-gray)] hover:bg-gray-100 transition-colors duration-300"
                  >
                    <div className="flex-shrink-0 w-10 h-10 rounded-lg bg-[var(--apple-blue)]/10 flex items-center justify-center">
                      <Icon className="w-5 h-5 text-[var(--apple-blue)]" />
                    </div>
                    <div>
                      <h4 className="font-semibold text-[var(--apple-dark)] mb-1">
                        {upgrade.title}
                      </h4>
                      <p className="text-sm text-[var(--apple-gray)]">
                        {upgrade.description}
                      </p>
                    </div>
                  </div>
                );
              })}
            </div>
          </div>
        </div>

        {/* 1Pro Section */}
        <div
          ref={proRef}
          className="grid lg:grid-cols-5 gap-8 lg:gap-12 items-center"
        >
          {/* Content */}
          <div className="lg:col-span-2 content-container order-2 lg:order-1">
            <h3 className="heading-subsection text-[var(--apple-dark)] mb-2">
              WuJin Calendar 1Pro
            </h3>
            <p className="price text-3xl font-bold text-[var(--apple-blue)] mb-6">
              ¥119.99
            </p>

            <div className="space-y-4">
              {upgrades1Pro.map((upgrade) => {
                const Icon = upgrade.icon;
                return (
                  <div
                    key={upgrade.title}
                    className="upgrade-item flex items-start gap-4 p-4 rounded-xl bg-[var(--apple-light-gray)] hover:bg-gray-100 transition-colors duration-300"
                  >
                    <div className="flex-shrink-0 w-10 h-10 rounded-lg bg-[var(--apple-blue)]/10 flex items-center justify-center">
                      <Icon className="w-5 h-5 text-[var(--apple-blue)]" />
                    </div>
                    <div>
                      <h4 className="font-semibold text-[var(--apple-dark)] mb-1">
                        {upgrade.title}
                      </h4>
                      <p className="text-sm text-[var(--apple-gray)]">
                        {upgrade.description}
                      </p>
                    </div>
                  </div>
                );
              })}
            </div>
          </div>

          {/* Image */}
          <div className="lg:col-span-3 product-image-container order-1 lg:order-2">
            <div className="relative overflow-hidden rounded-2xl">
              <img
                src="/images/product-black.png"
                alt="WuJin Calendar 1Pro"
                className="w-full h-auto"
              />
              <div className="absolute inset-0 bg-gradient-to-t from-black/20 to-transparent" />
            </div>
          </div>
        </div>
      </div>
    </section>
  );
};

export default Upgrades;
