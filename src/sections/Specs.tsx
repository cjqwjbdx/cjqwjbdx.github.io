import { useEffect, useRef } from 'react';
import { gsap } from 'gsap';
import { ScrollTrigger } from 'gsap/ScrollTrigger';
import { Check, X } from 'lucide-react';

gsap.registerPlugin(ScrollTrigger);

interface Spec {
  name: string;
  calendar1: string | boolean;
  calendar1Plus: string | boolean;
  calendar1Pro: string | boolean;
}

const specs: Spec[] = [
  { name: '价格', calendar1: '¥87.99', calendar1Plus: '¥97.99', calendar1Pro: '¥119.99' },
  { name: '电池', calendar1: '1000mAh', calendar1Plus: '1500mAh', calendar1Pro: '待定' },
  { name: '设计', calendar1: '手工焊接', calendar1Plus: 'PCB设计', calendar1Pro: 'PCB设计' },
  { name: '基本月历', calendar1: true, calendar1Plus: true, calendar1Pro: true },
  { name: '天气', calendar1: true, calendar1Plus: '天气预报', calendar1Pro: '天气预报' },
  { name: '倒数日&日期tag', calendar1: true, calendar1Plus: true, calendar1Pro: true },
  { name: '内存', calendar1: '4MB', calendar1Plus: '8MB', calendar1Pro: '8MB' },
  { name: '电池电量测量', calendar1: true, calendar1Plus: false, calendar1Pro: true },
  { name: '温湿度传感器', calendar1: false, calendar1Plus: false, calendar1Pro: true },
  { name: '超低功耗设计', calendar1: false, calendar1Plus: false, calendar1Pro: true },
  { name: '小程序配置', calendar1: false, calendar1Plus: false, calendar1Pro: true },
  { name: 'OTA自动升级', calendar1: true, calendar1Plus: true, calendar1Pro: true },
  { name: '自动配置', calendar1: false, calendar1Plus: true, calendar1Pro: true },
];

const Specs = () => {
  const sectionRef = useRef<HTMLElement>(null);
  const titleRef = useRef<HTMLDivElement>(null);
  const cardsRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const section = sectionRef.current;
    const title = titleRef.current;
    const cards = cardsRef.current;

    if (!section || !title || !cards) return;

    const ctx = gsap.context(() => {
      // Title animation
      gsap.fromTo(
        title,
        { opacity: 0, y: 30 },
        {
          opacity: 1,
          y: 0,
          duration: 0.6,
          ease: 'expo.out',
          scrollTrigger: {
            trigger: title,
            start: 'top 80%',
            toggleActions: 'play none none reverse',
          },
        }
      );

      // Cards animation
      const cardElements = cards.querySelectorAll('.spec-card-wrapper');
      cardElements.forEach((card, index) => {
        gsap.fromTo(
          card,
          { opacity: 0, y: 80, rotateX: 10 },
          {
            opacity: 1,
            y: 0,
            rotateX: 0,
            duration: 0.8,
            ease: 'expo.out',
            scrollTrigger: {
              trigger: cards,
              start: 'top 75%',
              toggleActions: 'play none none reverse',
            },
            delay: 0.2 + index * 0.15,
          }
        );
      });

      // Spec rows animation
      const specRows = cards.querySelectorAll('.spec-row');
      specRows.forEach((row, index) => {
        gsap.fromTo(
          row,
          { opacity: 0 },
          {
            opacity: 1,
            duration: 0.3,
            ease: 'expo.out',
            scrollTrigger: {
              trigger: cards,
              start: 'top 70%',
              toggleActions: 'play none none reverse',
            },
            delay: 0.5 + index * 0.05,
          }
        );
      });

      // Parallax on scroll
      gsap.to(cardElements, {
        y: -30,
        ease: 'none',
        scrollTrigger: {
          trigger: section,
          start: 'top bottom',
          end: 'bottom top',
          scrub: 0.5,
        },
      });

      // Center card scale effect
      gsap.to(cardElements[1], {
        scale: 1.02,
        ease: 'none',
        scrollTrigger: {
          trigger: section,
          start: 'top 50%',
          end: 'center center',
          scrub: 0.5,
        },
      });
    }, section);

    return () => ctx.revert();
  }, []);

  const renderSpecValue = (value: string | boolean) => {
    if (typeof value === 'boolean') {
      return value ? (
        <Check className="w-5 h-5 text-green-500 mx-auto" />
      ) : (
        <X className="w-5 h-5 text-gray-300 mx-auto" />
      );
    }
    return <span className="text-sm text-[var(--apple-dark)]">{value}</span>;
  };

  const productNames = ['WuJin Calendar 1', 'WuJin Calendar 1+', 'WuJin Calendar 1Pro'];
  const productPrices = ['¥87.99', '¥97.99', '¥119.99'];

  return (
    <section
      id="specs"
      ref={sectionRef}
      className="relative py-24 md:py-32 bg-white overflow-hidden"
    >
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        {/* Section Title */}
        <div ref={titleRef} className="text-center mb-16">
          <h2 className="heading-section text-[var(--apple-dark)] mb-4">
            技术规格
          </h2>
          <p className="body-large">详细对比，选择适合你的型号</p>
        </div>

        {/* Spec Cards */}
        <div
          ref={cardsRef}
          className="grid md:grid-cols-3 gap-6 lg:gap-8 perspective-800"
        >
          {[0, 1, 2].map((productIndex) => (
            <div
              key={productIndex}
              className={`spec-card-wrapper preserve-3d gpu-accelerate ${
                productIndex === 1 ? 'md:-mt-4' : ''
              }`}
            >
              <div
                className={`spec-card h-full ${
                  productIndex === 1
                    ? 'border-[var(--apple-blue)] border-2'
                    : ''
                }`}
              >
                {/* Card Header */}
                <div className="p-6 border-b border-gray-100">
                  <h3 className="text-lg font-semibold text-[var(--apple-dark)] mb-1">
                    {productNames[productIndex]}
                  </h3>
                  <p
                    className={`text-2xl font-bold ${
                      productIndex === 1
                        ? 'text-[var(--apple-blue)]'
                        : 'text-[var(--apple-dark)]'
                    }`}
                  >
                    {productPrices[productIndex]}
                  </p>
                </div>

                {/* Spec List */}
                <div className="p-6 space-y-4">
                  {specs.map((spec) => {
                    const value =
                      productIndex === 0
                        ? spec.calendar1
                        : productIndex === 1
                        ? spec.calendar1Plus
                        : spec.calendar1Pro;

                    return (
                      <div
                        key={spec.name}
                        className="spec-row flex items-center justify-between py-2 border-b border-gray-50 last:border-0"
                      >
                        <span className="text-sm text-[var(--apple-gray)]">
                          {spec.name}
                        </span>
                        {renderSpecValue(value)}
                      </div>
                    );
                  })}
                </div>

                {/* CTA */}
                <div className="p-6 pt-0">
                  <a
                    href="https://www.goofish.com/personal?spm=a21ybx.item.itemHeader.1.c03b3da6zaL1ag&userId=2219084601045"
                    target="_blank"
                    rel="noopener noreferrer"
                    className={`w-full py-3 rounded-full font-medium text-sm transition-all duration-300 flex items-center justify-center ${
                      productIndex === 1
                        ? 'bg-[var(--apple-blue)] text-white hover:bg-[var(--apple-blue-hover)]'
                        : 'bg-[var(--apple-light-gray)] text-[var(--apple-dark)] hover:bg-gray-200'
                    }`}
                  >
                    选择此型号
                  </a>
                </div>
              </div>
            </div>
          ))}
        </div>

        {/* Note */}
        <div className="mt-12 text-center">
          <p className="text-sm text-[var(--apple-gray)]">
            * 所有型号均支持农历、公共假期显示、天气、倒数日、自定义日期tag功能
          </p>
        </div>
      </div>
    </section>
  );
};

export default Specs;
