const aboutBtn = document.getElementById('aboutBtn');
const aboutModal = document.getElementById('aboutModal');
const closeModal = document.getElementById('closeModal');

aboutBtn.addEventListener('click', () => {
   aboutModal.style.display = 'flex';
});

closeModal.addEventListener('click', () => {
   aboutModal.style.display = 'none';
});

// close modal when clicking outside the modal content
aboutModal.addEventListener('click', (e) => {
   if (e.target === aboutModal) {
      aboutModal.style.display = 'none';
   }
});

const data = [
  { emoji: "✊", word: "ROCK" },
  { emoji: "✋", word: "PAPER" },
  { emoji: "✌️", word: "SCISSORS" }
];

const scr   = document.getElementById("scr");
const glyph = document.getElementById("glyph");
let timer;
const typeSound = new Audio("https://www.soundjay.com/communication/computer-keyboard-1.wav");

scr.textContent = "";

function play () {
  clearTimeout(timer);
  typeSound.pause();
  typeSound.currentTime = 0;

  const item = data[Math.random() * data.length | 0];
  let idx = 0;

  typeSound.loop = true;
  typeSound.play().catch(()=>{});

  (function flash () {
    if (idx < item.word.length) {
      scr.textContent = item.word[idx];
      idx++;
      timer = setTimeout(flash, 250);
    } else {
      typeSound.pause();
      typeSound.currentTime = 0;
      scr.textContent = item.emoji;
    }
  })();
}

glyph.addEventListener("click", play);


document.addEventListener('DOMContentLoaded', () => {

   // SPA navigation
   const links = document.querySelectorAll('nav a[href^="#"]');
   const sections = document.querySelectorAll('section');

   const showSection = (id) => {
      sections.forEach(s => s.classList.remove('active'));
      const target = document.getElementById(id);
      if (target) target.classList.add('active');
   };

   links.forEach(link => {
      link.addEventListener('click', (e) => {
         e.preventDefault();
         const href = link.getAttribute('href');
         if (href.startsWith('#')) {
            showSection(href.slice(1));
         }
      });
   });

   // Background toggle
   const bgToggle = document.getElementById('bgToggle');
   bgToggle.addEventListener('click', () => {
      document.body.classList.toggle('hideBg');
      bgToggle.textContent = document.body.classList.contains('hideBg') ? 'show bg' : 'hide bg';
   });

   // Neural network mouse repel
   const circles = document.querySelectorAll('.mesh circle[data-repel]');
   const lines = document.querySelectorAll('.mesh line');

   document.addEventListener('mousemove', (e) => {
      const rect = document.querySelector('.mesh').getBoundingClientRect();
      const mouseX = e.clientX - rect.left;
      const mouseY = e.clientY - rect.top;

      circles.forEach(circle => {
         const cx = parseFloat(circle.getAttribute('cx'));
         const cy = parseFloat(circle.getAttribute('cy'));
         const distance = Math.sqrt(Math.pow(mouseX - cx, 2) + Math.pow(mouseY - cy, 2));

         if (distance < 100) {
            const repelForce = (100 - distance) / 100;
            const angle = Math.atan2(cy - mouseY, cx - mouseX);
            const repelX = Math.cos(angle) * repelForce * 20;
            const repelY = Math.sin(angle) * repelForce * 20;

            circle.style.transform = `translate(${repelX}px, ${repelY}px)`;

            // Affect connected lines
            lines.forEach(line => {
               const x1 = parseFloat(line.getAttribute('x1'));
               const y1 = parseFloat(line.getAttribute('y1'));
               const x2 = parseFloat(line.getAttribute('x2'));
               const y2 = parseFloat(line.getAttribute('y2'));

               if ((x1 === cx && y1 === cy) || (x2 === cx && y2 === cy)) {
                  line.style.strokeOpacity = Math.max(0.05, 0.12 - repelForce * 0.1);
               }
            });
         } else {
            circle.style.transform = '';
            lines.forEach(line => {
               line.style.strokeOpacity = '';
            });
         }
      });
   });

   // Uiverse switch
   const switchInput = document.getElementById('switchInput');
   const switchContainer = document.getElementById('switchContainer');
   const switchLabel = document.getElementById('switchLabel');

   switchInput.addEventListener('change', () => {
      const isChecked = switchInput.checked;
      switchContainer.classList.toggle('checked', isChecked);
      switchLabel.classList.toggle('checked', isChecked);
      document.body.classList.toggle('alt', isChecked);

      // Play sound
      const sound = document.getElementById(isChecked ? 'soundOn' : 'soundOff');
      sound.play().catch(e => console.log('Sound play failed:', e));
   });

   // Spinner
   const spinner = document.getElementById('spinner');

   spinner.addEventListener('click', () => {
      const duration = 3000; // 3 seconds spin
      const easingFrames = 60;
      let angle = 0;
      let frame = 0;

      const animateSpin = () => {
         const t = frame / easingFrames;
         const eased = Math.pow(1 - t, 3); // cubic ease-out
         angle += eased * 20;
         spinner.style.transform = `rotate(${angle}deg)`;

         frame++;
         if (frame <= easingFrames) {
            requestAnimationFrame(animateSpin);
         }
      };

      animateSpin();
   });

   // Card door
   const door = document.getElementById('cardDoor');
   let cardOpen = false;

   door.addEventListener('click', () => {
      cardOpen = !cardOpen;
      door.classList.toggle('open', cardOpen);

      const sound = document.getElementById(cardOpen ? 'soundCardOpen' : 'soundCardClose');
      sound.currentTime = 0;
      sound.play().catch(e => console.log('Card sound failed:', e));
   });
});