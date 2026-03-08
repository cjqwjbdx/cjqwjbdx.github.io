import { useEffect, useRef } from 'react';
import { gsap } from 'gsap';
import { ScrollTrigger } from 'gsap/ScrollTrigger';
import { Calendar, Cloud, Timer, Tag } from 'lucide-react';

gsap.registerPlugin(ScrollTrigger);

interface Feature {
  icon: React.ElementType;
  title: string;
  description: string;
}

const features: Feature[] = [
  {
    icon: Calendar,
    title: '基本月历信息',
    description: '支持农历、公共假期显示，时间一目了然',
  },
  {
    icon: Cloud,
    title: '天气',
    description: '实时天气、每日天气、湿度、风向、风速，随时掌握',
  },
  {
    icon: Timer,
    title: '倒数日',
    description: '重要的日期标记，还剩多少天一清二楚',
  },
  {
    icon: Tag,
    title: '自定义日期tag',
    description: '标记重要且重复的日子，如发工资的日子',
  },
];

const Features = () => {
  const sectionRef = useRef<HTMLElement>(null);
  const contentRef = useRef<HTMLDivElement>(null);
  const imageRef = useRef<HTMLDivElement>(null);
  const featuresRef = useRef<HTMLDivElement>(null);
  const openSourceRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const section = sectionRef.current;
    const content = contentRef.current;
    const image = imageRef.current;
    const featuresEl = featuresRef.current;
    const openSource = openSourceRef.current;

    if (!section || !content || !image || !featuresEl || !openSource) return;

    const ctx = gsap.context(() => {
      // Content animation
      gsap.fromTo(
        content.querySelector('h2'),
        { opacity: 0, x: -50 },
        {
          opacity: 1,
          x: 0,
          duration: 0.6,
          ease: 'expo.out',
          scrollTrigger: {
            trigger: content,
            start: 'top 80%',
            toggleActions: 'play none none reverse',
          },
        }
      );

      // Feature items animation
      const featureItems = featuresEl.querySelectorAll('.feature-item');
      featureItems.forEach((item, index) => {
        gsap.fromTo(
          item,
          { opacity: 0, x: -30 },
          {
            opacity: 1,
            x: 0,
            duration: 0.4,
            ease: 'expo.out',
            scrollTrigger: {
              trigger: featuresEl,
              start: 'top 75%',
              toggleActions: 'play none none reverse',
            },
            delay: index * 0.1,
          }
        );
      });

      // Image animation
      gsap.fromTo(
        image,
        { opacity: 0, y: 80 },
        {
          opacity: 1,
          y: 0,
          duration: 0.8,
          ease: 'expo.out',
          scrollTrigger: {
            trigger: image,
            start: 'top 80%',
            toggleActions: 'play none none reverse',
          },
        }
      );

      // Open source text typewriter effect
      gsap.fromTo(
        openSource,
        { opacity: 0 },
        {
          opacity: 1,
          duration: 0.8,
          ease: 'none',
          scrollTrigger: {
            trigger: openSource,
            start: 'top 85%',
            toggleActions: 'play none none reverse',
          },
        }
      );

      // Parallax on scroll
      gsap.to(image, {
        y: -60,
        ease: 'none',
        scrollTrigger: {
          trigger: section,
          start: 'top bottom',
          end: 'bottom top',
          scrub: 0.5,
        },
      });

      gsap.to(content, {
        y: 30,
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
      id="features"
      ref={sectionRef}
      className="relative py-24 md:py-32 overflow-hidden"
      style={{
        backgroundColor: 'var(--apple-light-gray)',
        transform: 'skewY(-3deg)',
      }}
    >
      <div
        className="relative z-10 max-w-7xl mx-auto px-4 sm:px-6 lg:px-8"
        style={{ transform: 'skewY(3deg)' }}
      >
        <div className="grid lg:grid-cols-2 gap-12 lg:gap-16 items-center">
          {/* Left: Content */}
          <div ref={contentRef}>
            <h2 className="heading-section text-[var(--apple-dark)] mb-8">
              强大功能
              <br />
              <span className="text-gradient">一应俱全</span>
            </h2>

            <div ref={featuresRef} className="space-y-6">
              {features.map((feature) => {
                const Icon = feature.icon;
                return (
                  <div
                    key={feature.title}
                    className="feature-item flex items-start gap-4 p-4 rounded-xl bg-white/50 hover:bg-white transition-colors duration-300 group"
                  >
                    <div className="flex-shrink-0 w-12 h-12 rounded-xl bg-[var(--apple-blue)]/10 flex items-center justify-center group-hover:bg-[var(--apple-blue)]/20 transition-colors duration-300">
                      <Icon className="w-6 h-6 text-[var(--apple-blue)]" />
                    </div>
                    <div>
                      <h3 className="text-lg font-semibold text-[var(--apple-dark)] mb-1">
                        {feature.title}
                      </h3>
                      <p className="text-[var(--apple-gray)]">
                        {feature.description}
                      </p>
                    </div>
                  </div>
                );
              })}
            </div>

            {/* Open Source Note */}
            <div
              ref={openSourceRef}
              className="mt-8 p-4 rounded-xl bg-[var(--apple-dark)] text-white"
            >
              <p className="text-sm font-medium">
                WuJin Calendar 1基于开源项目J-Calendar
              </p>
            </div>
          </div>

          {/* Right: Image */}
          <div ref={imageRef} className="flex justify-center">
            <div className="relative">
              <img
                src="/images/product-black.png"
                alt="WuJin Calendar Features"
                className="w-full max-w-lg h-auto rounded-2xl shadow-2xl transition-transform duration-500 hover:scale-[1.02]"
              />
              {/* Decorative Elements */}
              <div className="absolute -top-4 -right-4 w-24 h-24 rounded-full bg-[var(--apple-blue)]/10 blur-xl" />
              <div className="absolute -bottom-4 -left-4 w-32 h-32 rounded-full bg-[var(--apple-blue)]/5 blur-xl" />
            </div>
          </div>
        </div>
      </div>
    </section>
  );
};

export default Features;
