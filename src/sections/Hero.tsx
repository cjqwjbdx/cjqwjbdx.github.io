import { useEffect, useRef } from 'react';
import { gsap } from 'gsap';
import { ScrollTrigger } from 'gsap/ScrollTrigger';
import { ChevronRight } from 'lucide-react';

gsap.registerPlugin(ScrollTrigger);

const Hero = () => {
  const sectionRef = useRef<HTMLElement>(null);
  const titleRef = useRef<HTMLDivElement>(null);
  const subtitleRef = useRef<HTMLParagraphElement>(null);
  const productRef = useRef<HTMLDivElement>(null);
  const ctaRef = useRef<HTMLDivElement>(null);
  const bgRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const section = sectionRef.current;
    const title = titleRef.current;
    const subtitle = subtitleRef.current;
    const product = productRef.current;
    const cta = ctaRef.current;
    const bg = bgRef.current;

    if (!section || !title || !subtitle || !product || !cta || !bg) return;

    const ctx = gsap.context(() => {
      // Initial states
      gsap.set([title.querySelector('.title-wujin'), title.querySelector('.title-calendar')], {
        opacity: 0,
        x: (i) => (i === 0 ? -100 : 100),
      });
      gsap.set(subtitle, { opacity: 0, y: 20 });
      gsap.set(product, { opacity: 0, rotateY: 90 });
      gsap.set(cta.children, { opacity: 0, scale: 0 });
      gsap.set(bg, { opacity: 0 });

      // Entry timeline
      const tl = gsap.timeline({ delay: 0.3 });

      tl.to(bg, {
        opacity: 1,
        duration: 1.2,
        ease: 'expo.out',
      })
        .to(
          title.querySelector('.title-wujin'),
          {
            opacity: 1,
            x: 0,
            duration: 0.8,
            ease: 'expo.out',
          },
          '-=0.9'
        )
        .to(
          title.querySelector('.title-calendar'),
          {
            opacity: 1,
            x: 0,
            duration: 0.8,
            ease: 'expo.out',
          },
          '-=0.6'
        )
        .to(
          product,
          {
            opacity: 1,
            rotateY: 0,
            duration: 1,
            ease: 'expo.out',
          },
          '-=0.5'
        )
        .to(
          subtitle,
          {
            opacity: 1,
            y: 0,
            duration: 0.6,
            ease: 'expo.out',
          },
          '-=0.5'
        )
        .to(
          cta.children,
          {
            opacity: 1,
            scale: 1,
            duration: 0.5,
            stagger: 0.1,
            ease: 'back.out(1.7)',
          },
          '-=0.3'
        );

      // Scroll animations
      const scrollTl = gsap.timeline({
        scrollTrigger: {
          trigger: section,
          start: 'top top',
          end: 'bottom top',
          scrub: 0.5,
        },
      });

      scrollTl
        .to(product, {
          y: -80,
          scale: 1.1,
          ease: 'none',
        })
        .to(
          title.querySelector('.title-wujin'),
          {
            x: -50,
            ease: 'none',
          },
          0
        )
        .to(
          title.querySelector('.title-calendar'),
          {
            x: 50,
            ease: 'none',
          },
          0
        )
        .to(
          bg,
          {
            opacity: 0.3,
            ease: 'none',
          },
          0
        )
        .to(
          section,
          {
            opacity: 0,
            ease: 'none',
          },
          0.5
        );

      // Floating animation for product
      gsap.to(product, {
        y: '+=10',
        duration: 3,
        repeat: -1,
        yoyo: true,
        ease: 'sine.inOut',
      });
    }, section);

    return () => ctx.revert();
  }, []);

  const scrollToSection = (href: string) => {
    const element = document.querySelector(href);
    if (element) {
      element.scrollIntoView({ behavior: 'smooth' });
    }
  };

  return (
    <section
      id="hero"
      ref={sectionRef}
      className="relative min-h-screen flex items-center justify-center overflow-hidden bg-white"
    >
      {/* Background Gradient */}
      <div
        ref={bgRef}
        className="absolute inset-0 opacity-0"
        style={{
          background: 'radial-gradient(ellipse at 50% 30%, rgba(245,245,247,1) 0%, rgba(255,255,255,0) 70%)',
        }}
      />

      {/* Content Container */}
      <div className="relative z-10 w-full max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 pt-20 pb-16">
        <div className="grid lg:grid-cols-2 gap-8 lg:gap-12 items-center">
          {/* Left: Text Content */}
          <div className="text-center lg:text-left order-2 lg:order-1">
            {/* Title */}
            <div ref={titleRef} className="mb-6">
              <h1 className="heading-hero text-[var(--apple-dark)]">
                <span className="title-wujin inline-block">WuJin</span>
                <br />
                <span className="title-calendar inline-block">Calendar</span>
              </h1>
            </div>

            {/* Subtitle */}
            <p
              ref={subtitleRef}
              className="body-large mb-8 max-w-md mx-auto lg:mx-0"
            >
              匠心科技，融入生活
              <br />
              <span className="text-[var(--apple-gray)]">
                一款完美呈现时间的智能墨水屏日历
              </span>
            </p>

            {/* CTA Buttons */}
            <div ref={ctaRef} className="flex flex-col sm:flex-row gap-4 justify-center lg:justify-start">
              <a
                href="https://www.goofish.com/personal?spm=a21ybx.item.itemHeader.1.c03b3da6zaL1ag&userId=2219084601045"
                target="_blank"
                rel="noopener noreferrer"
                className="btn-primary"
              >
                立即购买
              </a>
              <button
                onClick={() => scrollToSection('#products')}
                className="btn-text group"
              >
                了解更多
                <ChevronRight className="w-4 h-4 ml-1 transition-transform group-hover:translate-x-1" />
              </button>
            </div>
          </div>

          {/* Right: Product Image */}
          <div className="order-1 lg:order-2 flex justify-center perspective-1000">
            <div
              ref={productRef}
              className="relative preserve-3d gpu-accelerate"
              style={{
                transform: 'rotateY(-5deg) rotateX(2deg)',
              }}
            >
              <img
                src="/images/product-white.png"
                alt="WuJin Calendar"
                className="w-full max-w-lg h-auto rounded-2xl shadow-2xl"
              />
              {/* Glow Effect */}
              <div
                className="absolute -inset-4 -z-10 rounded-3xl opacity-30 blur-2xl"
                style={{
                  background: 'linear-gradient(135deg, rgba(0,113,227,0.2) 0%, rgba(0,102,204,0.1) 100%)',
                }}
              />
            </div>
          </div>
        </div>
      </div>

      {/* Decorative Elements */}
      <div className="absolute bottom-0 left-0 right-0 h-32 bg-gradient-to-t from-white to-transparent pointer-events-none" />
    </section>
  );
};

export default Hero;
