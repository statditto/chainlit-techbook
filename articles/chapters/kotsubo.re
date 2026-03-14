
= AIが今何をしているか? で不安にならないようにしよう！

//lead{
hogehoge
//}

== 本章で利用するアプリケーション

本章では @<fn>{support} を例に、ChainlitのStep機能を中心に説明します。ユーザーはブラウザからアクセスしてチャットを行います。

//footnote[support][本章のソースコードは以下から参照できます。@<href>{https://github.com/takuto0831/chainlit-playground}]


== 概要

Chainlit には `cl.Step` というコンポーネントがあり、LLM アプリケーションの処理過程をユーザーに可視化できます。本記事では、OpenAI の web_search_preview ツールを使ったリサーチツールを題材に、Step の基本実装から始め、4つの方向でUI/UXを段階的に改善する実装例を紹介します。


== 背景

llmの精度が高まり、Toolによる機能の拡張が増えると一度の応答にかかる時間が非常に長くなる傾向があります。思考の過程を何も表示しないとユーザーは本当に動いているのか？という不安を抱く可能性もあります。また長時間考えて返ってきた結果が意図するものと大きく異なるケースなどもユーザーの体験としてあまり良くないです。
思考の過程を分かりやすく表示する工夫や待ち時間を有効にする工夫を調査しました。

過程を何も表示しないと、このような画面が表示されるだけなので本当に処理が進んでいるか少し不安になると思います。（@<img>{kotsubo/no_step}）
//image[kotsubo/no_step][過程を表示しない実行例][scale=0.8]{
//}

=== この記事でわかること

- Chainlit のStep表示の紹介
- Step表示におけるformatの簡単な変更方法
- ChainlitのTaskListやPlotlyの活用

=== `@step`って何をする機能?

Chainlitには **Step（ステップ）** という概念があります。
処理の「中間状態」をUIに表示するための仕組みで、LLMが裏で何をしているかをユーザーに見せたいときに使います。

基本的な使い方はこんな感じ：

//emlist[基本的な使い方]{
import asyncio
import chainlit as cl

@cl.on_message
async def main(message: cl.Message):
    await process(message.content)

@cl.step
async def process(content: str):
    # ここの処理がUIにステップとして表示される
    await asyncio.sleep(1)
    return f"処理完了: {content}"
//}

実行すると、チャット画面に「process」というラベルがついた折りたたみUIが現れます。
「あ、裏でこういう処理が走ってるんだ」が一目でわかる。かっこいい。

== Stepを利用した

//emlist[ベースの実装の一部]{
@cl.on_message
async def main(message: cl.Message) -> None:
    query = message.content
    all_findings: list[str] = []

    async with cl.Step(name="🔎 ウェブを調査しています", type="tool") as root_step:
        root_step.input = query

        topics = await generate_topics(query)

        for topic in topics:
            async with cl.Step(
                name=f"🔍「{topic}」を調査中", type="tool"
            ) as topic_step:
                topic_step.input = f"「{topic}」の観点で調査"

                sites = await research_topic(query, topic)

                for site in sites:
                    async with cl.Step(
                        name=f"📄 {site['name']}", type="retrieval"
                    ) as site_step:
                        site_step.input = site["url"]
                        site_step.output = site["summary"]

                    all_findings.append(
                        f"**[{topic}｜{site['name']}]** {site['summary']}"
                    )

                topic_step.output = f"{len(sites)} 件のソースを確認しました"

        root_step.output = f"合計 {len(all_findings)} 件のソースを調査しました"

    answer_msg = cl.Message(content="")
    await answer_msg.send()
    await aggregate(query, all_findings, answer_msg)
    await answer_msg.update()
//}

=== stepの入れ子表示について

//emlist[stepの入れ子表示]{
async with cl.Step(name="親", type="tool") as parent:
    async with cl.Step(name="子", type="retrieval") as child:
        child.output = "子の結果"
    parent.output = "親の結果"
//}

=== 実行例

//emlist[実行例]{
 make run TARGET=step_child_base
//}
（@<img>{kotsubo/ui_base}）
//image[kotsubo/ui_base][基本的な Step利用の表示例][scale=0.8]{
//}


== 改善1: Markdown によるStep出力の構造化

//emlist[Markdownによる]{
def fmt_topic_output(_topic: str, sites: list[dict]) -> str:
"""トピックstepのoutput：サイト名・ドメイン・信頼度のみ表示

After:
    | ソース | ドメイン | 信頼度 |
    |--------|----------|--------|
    | Wikipedia | ja.wikipedia.org | ⭐⭐⭐⭐ |
    ...
    > 🔍 **2 件**のソースを確認しました
"""
rows = "\n".join(
    f"| [{s['name']}]({s['url']}) | `{urlparse(s['url']).netloc}`"
    f" | {s['reliability']} |"
    for s in sites
)
return (
    f"| ソース | ドメイン | 信頼度 |\n"
    f"|--------|----------|--------|\n"
    f"{rows}\n\n"
    f"> 🔍 **{len(sites)} 件**のソースを確認しました"
)
//}

=== なぜ Markdown テーブルが有効か

- **1クリックで全ソースが見える**：`site_step` を個別に展開しなくても、トピックの `output` 一覧だけで調査先と信頼度が分かる

- **ソース名がリンク化**：クリックで実際のURLに飛べる

- **ドメインをコードブロックで表示**：`` `ja.wikipedia.org` `` の形式でソースの出所が一目で判断できる

- **信頼度が視覚的**：⭐ の数でスキャンしやすい

- **blockquote でサマリー**：`>` 記法でカウント情報を目立たせている

（@<img>{kotsubo/ui_markdown}）,（@<img>{kotsubo/ui_markdown2}
）
//image[kotsubo/ui_markdown][hogehoge][scale=0.8]{
//}
//image[kotsubo/ui_markdown2][hogehoge][scale=0.8]{
//}

== 改善2: TaskList によるサイドバー進捗表示

`cl.TaskList` と `cl.Task` を追加しました。Step のツリー表示と並行して、チャット右側のサイドバーにタスクリストが表示され、各トピックの進捗状況（実行中 / 完了）をリアルタイムで確認できます。

（@<img>{kotsubo/ui_tasklist}）,（@<img>{kotsubo/ui_tasklist2}
）
//image[kotsubo/ui_tasklist][hogehoge][scale=0.8]{
//}

//image[kotsubo/ui_tasklist2][hogehoge][scale=0.8]{
//}

Step は処理の詳細（入力・出力・階層）を展開して確認し、TaskListは全体の進捗を一覧でリアルタイム把握します。

Step ツリーは処理の「詳細」を見るものであり、全体進捗の把握には向きません。特にトピック数が増えたとき、どこまで終わっているかが一目で分かりません。TaskList をサイドバーに置くことで：

=== なぜ TaskList が有効か
- **スクロールせずに全体進捗が見える**（サイドバーは常時表示）

- **「今どのトピックを処理中か」が明確**

- **完了数 / 総数のカウントが分かりやすい**

== 改善3: Plotly ヒートマップによる信頼度の可視化

全トピックの調査完了後に Plotly のインタラクティブなヒートマップチャートを `cl.Plotly` で表示します。信頼度情報を数値化して色で表現することで、「どのトピックのどのソースが高品質か」を視覚的に把握できます。（@<img>{kotsubo/ui_chart}

//image[kotsubo/ui_chart][hogehoge][scale=0.8]{
//}


== 改善4: 豆知識の並行表示

`asyncio.gather` を使って「トピック生成」と「豆知識生成」を並行実行し、リサーチ待機中にユーザーへ関連情報を表示します。

//emlist[豆知識の生成関数]{
async def generate_trivia(query: str) -> str:
    """クエリに関連する面白い豆知識を1つ生成する。"""
    response = await client.chat.completions.create(
        model="gpt-4o-mini",
        max_tokens=256,
        messages=[
            {
                "role": "system",
                "content": (
                    "あなたは博識なアシスタントです。"
                    "与えられたテーマに関連する、知っていると少し得する面白い豆知識を1つ、"
                    "日本語で3〜5文程度で教えてください。"
                    "「💡 豆知識：」で始めてください。"
                ),
            },
            {
                "role": "user",
                "content": f"「{query}」に関連する豆知識を1つ教えてください。",
            },
        ],
    )
    return response.choices[0].message.content or ""
//}

トピック生成が完了した時点で豆知識も揃っており、即時表示できます（@<img>{kotsubo/ui_trivia}

//image[kotsubo/ui_trivia][hogehoge][scale=0.8]{
//}

豆知識を表示して、ユーザに読んでもらうことで、ユーザの待ち時間を退屈させない工夫です

== まとめ

Step機能の拡張や関連機能を使うことでユーザの待ち時間の不安を和らげる方法について幾つか紹介しました。

* **Markdown**：最小コストで Step 出力を人間が読みやすくしたいとき
* **TaskList**：処理ステップが多く、全体進捗管理が重要なとき
* **Plotly**：数値・スコアデータを比較・可視化したいとき
* **Trivia（asyncio.gather）**：待機時間が長いアプリで、UX 上の「間」を埋めたいとき
