import { useEffect, useRef } from 'react';
import { gsap } from 'gsap';
import { ScrollTrigger } from 'gsap/ScrollTrigger';

gsap.registerPlugin(ScrollTrigger);

interface Product {
  name: string;
  price: string;
  image: string;
  description: string;
}

const products: Product[] = [
  {
    name: 'WuJin Calendar 1',
    price: '¥87.99',
    image: '/images/product-white.png',
    description: '手工焊接设计 · 1000mAh电池',
  },
  {
    name: 'WuJin Calendar 1+',
    price: '¥97.99',
    image: '/images/product-black.png',
    description: 'PCB设计 · 1500mAh电池 · 天气预报',
  },
  {
    name: 'WuJin Calendar 1Pro',
    price: '¥119.99',
    image: '/images/product-black.png',
    description: 'PCB设计 · 温湿度传感器 · 小程序配置',
  },
];

const Products = () => {
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
          duration: 0.8,
          ease: 'expo.out',
          scrollTrigger: {
            trigger: title,
            start: 'top 80%',
            toggleActions: 'play none none reverse',
          },
        }
      );

      // Cards animation with 3D effect
      const cardElements = cards.querySelectorAll('.product-card-wrapper');
      cardElements.forEach((card, index) => {
        const isLeft = index === 0;
        const isRight = index === 2;
        const isCenter = index === 1;

        gsap.fromTo(
          card,
          {
            opacity: 0,
            x: isLeft ? -200 : isRight ? 200 : 0,
            y: isCenter ? 100 : 0,
            rotateY: isLeft ? 45 : isRight ? -45 : 0,
            scale: isCenter ? 0.8 : 1,
          },
          {
            opacity: 1,
            x: 0,
            y: 0,
            rotateY: isLeft ? 15 : isRight ? -15 : 0,
            scale: isCenter ? 1.05 : 1,
            duration: 1,
            ease: 'expo.out',
            scrollTrigger: {
              trigger: cards,
              start: 'top 75%',
              toggleActions: 'play none none reverse',
            },
            delay: index * 0.2,
          }
        );
      });

      // Parallax on scroll
      gsap.to(cardElements, {
        x: -50,
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
      id="products"
      ref={sectionRef}
      className="relative py-24 md:py-32 bg-black overflow-hidden"
    >
      {/* Spotlight Effect */}
      <div
        className="absolute inset-0 pointer-events-none"
        style={{
          background: 'radial-gradient(ellipse at 50% 50%, rgba(255,255,255,0.03) 0%, transparent 50%)',
        }}
      />

      <div className="relative z-10 max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        {/* Section Title */}
        <div ref={titleRef} className="text-center mb-16">
          <h2 className="heading-section text-white mb-4">选择你的风格</h2>
          <p className="body-large text-gray-400">三种型号，无限可能</p>
        </div>

        {/* Products Grid */}
        <div
          ref={cardsRef}
          className="grid md:grid-cols-3 gap-6 lg:gap-8 perspective-1200"
        >
          {products.map((product, index) => (
            <div
              key={product.name}
              className="product-card-wrapper preserve-3d gpu-accelerate"
              style={{
                transform:
                  index === 0
                    ? 'translateZ(-100px) rotateY(15deg)'
                    : index === 2
                    ? 'translateZ(-100px) rotateY(-15deg)'
                    : 'translateZ(0) scale(1.05)',
              }}
            >
              <div className="product-card bg-[#1D1D1F] border border-gray-800">
                {/* Product Image */}
                <div className="relative aspect-[4/3] overflow-hidden bg-gradient-to-b from-gray-800 to-[#1D1D1F]">
                  <img
                    src={product.image}
                    alt={product.name}
                    className="w-full h-full object-cover transition-transform duration-500 hover:scale-105"
                  />
                </div>

                {/* Product Info */}
                <div className="p-6 text-center">
                  <h3 className="text-xl font-semibold text-white mb-2">
                    {product.name}
                  </h3>
                  <p className="text-2xl font-bold text-[var(--apple-blue)] mb-3">
                    {product.price}
                  </p>
                  <p className="text-sm text-gray-400 mb-4">{product.description}</p>
                  <a
                    href="https://www.goofish.com/personal?spm=a21ybx.item.itemHeader.1.c03b3da6zaL1ag&userId=2219084601045"
                    target="_blank"
                    rel="noopener noreferrer"
                    className="inline-flex items-center justify-center px-6 py-2 bg-[var(--apple-blue)] text-white text-sm font-medium rounded-full hover:bg-[var(--apple-blue-hover)] transition-all duration-300 hover:scale-105"
                  >
                    立即购买
                  </a>
                </div>
              </div>
            </div>
          ))}
        </div>
      </div>
    </section>
  );
};

export default Products;
